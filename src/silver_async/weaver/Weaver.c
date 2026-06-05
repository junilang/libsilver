#ifndef Weaver_SAFE
	#define Weaver_SAFE BUILD_SAFE
#endif

#ifndef Weaver_DEBUG
	#define Weaver_DEBUG false //BUILD_DEBUG
#endif

#ifndef Weaver_CALL_IMMEDIATE
	#define Weaver_CALL_IMMEDIATE true
#endif

#ifndef Weaver_RESUME_IMMEDIATE
	#define Weaver_RESUME_IMMEDIATE true
#endif

#if Weaver_DEBUG
	_Thread_local u32 t_WeaverThread_id = 0;

	#define Weaver_DBG(...) FPRINT_ATOMIC( \
		stderr, "[",t_WeaverThread_id,"] ",__func__,": "__VA_OPT__(,)__VA_ARGS__,"\n" \
	)
#else
	#define Weaver_DBG(...)
#endif

#include "WeaverQueue.h"

#define Weaver_MAX_THREADS ((1ull << WeaverQinfo_RC_BITS) - 1)

#include "WeaverThread.h"
#include "Weaver.h"

Ptr WeaverThread_main(Ptr);

void WeaverThread_boot(Weaver *rt, u16 id) {
	WeaverThread *this = &rt->threads[id];
	this->state = WeaverThread_BOOT + id;
	pthread_create(&this->thread, nullptr, &WeaverThread_main, this);
}

void Weaver_boot(Weaver *this) {
	const u16 size = this->threads_size;

	atomic_store_explicit(&this->threads_sync, size, memory_order_release);
	atomic_store_explicit(&this->lock, Weaver_LOCK_JOIN, memory_order_release);

	for (u16 id = 0; id < size; id++) {
		WeaverThread_boot(this, id);
	}

	do {
		switch (umtx_wait(&this->lock, Weaver_LOCK_JOIN)) {
			case umtxResult_MISMATCH:
				goto break_on_mismatch;
			default:;
		}
	} while (
		atomic_load_explicit(&this->lock, memory_order_relaxed)
			== Weaver_LOCK_JOIN
	);

	break_on_mismatch:;

	#if 0 && Weaver_SAFE
		if (atomic_load_explicit(&this->threads_sync, memory_order_acquire) != 0)
			PANIC("boot thread sync failed");
	#endif
}

void Weaver_wake(Weaver *this, usize n) {
	auto it = this->threads;
	auto const end = it + this->threads_size;

	for (; n && (it < end); it++) {
		umtx want = WeaverThread_IDLE;
		if (
			atomic_compare_exchange_strong_explicit(
				&it->state, &want, WeaverThread_RUN,
				memory_order_relaxed, memory_order_relaxed
			)
		) {
			umtx_wake_all(&it->state);
			n--;
		}
	}
}

// TODO
void Weaver_join(Weaver *this) {
	umtx want = Weaver_LOCK_NONE;

	if (
		!atomic_compare_exchange_strong(
			&this->lock, &want, Weaver_LOCK_JOIN
		)
	)
		PANIC("invalid lock state");

	// wait for all threads to be idle
	auto const sync = atomic_load_explicit(&this->threads_sync, memory_order_acquire);
	if (sync == 0) {
		atomic_store_explicit(&this->lock, Weaver_LOCK_NONE, memory_order_release);
		goto down;
	}

	do {
		switch (umtx_wait(&this->lock, Weaver_LOCK_JOIN)) {
			case umtxResult_MISMATCH:
				goto down;
			default:
		}
	} while (atomic_load_explicit(&this->lock, memory_order_relaxed) == Weaver_LOCK_JOIN);

	down:;
	want = Weaver_LOCK_NONE;
	if (
		!atomic_compare_exchange_strong_explicit(
			&this->lock, &want, Weaver_LOCK_DOWN,
			memory_order_acq_rel, memory_order_relaxed
		)
	)
		PANIC("invalid lock state");

	// wake up all threads and set their state to down
	auto it = this->threads;
	auto const end = it + this->threads_size;
	for (; it < end; it++) {
		if (
			atomic_exchange_explicit(
				&it->state, WeaverThread_DOWN, memory_order_acq_rel
			) == WeaverThread_IDLE
		) {
			umtx_wake_all(&it->state);
		}
	}

	do {
		switch (umtx_wait(&this->lock, Weaver_LOCK_DOWN)) {
			case umtxResult_MISMATCH:
				goto break_on_mismatch;
			default:
		}
	} while (
		atomic_load_explicit(&this->lock, memory_order_relaxed) == Weaver_LOCK_DOWN
	);

	break_on_mismatch:;

	// join threads using pthreads interface
	it = this->threads;
	for (; it < end; it++) {
		pthread_join(it->thread, nullptr);
	}
}

// returns true if the calling thread successfully performed a swap
bool Weaver_swap(Weaver *this, WeaverQinfo expected_index) {
	WeaverQinfo qinfo = atomic_fetch_or_explicit(
		&this->qinfo, WeaverQinfo_SWAP_LOCK, memory_order_acq_rel
	);

	if (qinfo & WeaverQinfo_SWAP_LOCK) {
		// another thread started swapping before us
		return false;
	}

	// if the expected index provided by caller does not match the current index
	// a swap operation already must have happened between when the caller
	// decided a swap was necessary and we acquired our lock
	if ((qinfo ^ expected_index) & WeaverQinfo_INDEX_MASK) {
		// release lock
		atomic_fetch_and_explicit(&this->qinfo, ~WeaverQinfo_SWAP_LOCK, memory_order_acquire);
		return true;
	}

	// acquire mqinfo lock
	WeaverQinfo mqinfo = atomic_fetch_or_explicit(
		&this->mqinfo, WeaverQinfo_SWAP_LOCK, memory_order_acq_rel
	);

	// wait until other threads release references
	while (mqinfo & (WeaverQinfo_RC_MASK | WeaverQinfo_RESIZE_LOCK)) {
		CPU_YIELD;
		mqinfo = atomic_load_explicit(&this->mqinfo, memory_order_acquire);
	}

	// qinfo doesn't use the resize lock
	while (qinfo & WeaverQinfo_RC_MASK) {
		CPU_YIELD;
		qinfo = atomic_load_explicit(&this->qinfo, memory_order_acquire);
	}

	// we now have both locks and no valid references are held by other threads

	// set queue size
	const usize size = WeaverQinfo_pos(mqinfo);
	this->queue[mqinfo & WeaverQinfo_INDEX_BIT]->size = size;

	// perform swap
	atomic_store_explicit(
		&this->mqinfo, (mqinfo + 1) & WeaverQinfo_INDEX_MASK, memory_order_release
	);
	atomic_store_explicit(
		&this->qinfo, (qinfo + 1) & WeaverQinfo_INDEX_MASK, memory_order_release
	);

	// wake threads
	Weaver_wake(this, size);
	return true;
}

void Weaver_submit(Ptr vthis, const AsyncTask *tasks, usize tasks_size) {
	Weaver *const this = vthis;

	acquire_reference:;
	WeaverQinfo mqinfo = atomic_fetch_add_explicit(
		&this->mqinfo, WeaverQinfo_RC_ONE, memory_order_acquire
	);

	// if another thread is performing a swap
	if (mqinfo & WeaverQinfo_SWAP_LOCK) {
		WeaverQinfo mqinfo_0 = mqinfo;
		while (true) {
			// try to release our reference
			if (
				atomic_compare_exchange_weak_explicit(
					&this->mqinfo, &mqinfo, mqinfo - WeaverQinfo_RC_ONE,
					memory_order_acquire, memory_order_acquire
				)
			)
				break;

			// if the swap completed without releasing our reference,
			// the reference was lost
			if ((mqinfo ^ mqinfo_0) & WeaverQinfo_INDEX_MASK)
				goto acquire_reference;
		}

		// wait until swap completes
		while (!((mqinfo ^ mqinfo_0) & WeaverQinfo_INDEX_MASK)) {
			CPU_YIELD;
			mqinfo = atomic_load_explicit(&this->mqinfo, memory_order_relaxed);
		}

		goto acquire_reference;
	}

	if (mqinfo & WeaverQinfo_RESIZE_LOCK) {
		yield_to_resize_lock:;

		while (true) {
			if (
				atomic_compare_exchange_weak_explicit(
					&this->mqinfo, &mqinfo, mqinfo - WeaverQinfo_RC_ONE,
					memory_order_acquire, memory_order_acquire
				)
			)
				break;

			// if the resize completes before we release our reference,
			// the reference is still valid and we can continue
			if (!(mqinfo & WeaverQinfo_RESIZE_LOCK))
				goto has_reference;
		}

		// wait for swap to complete and reacquire reference
		while (mqinfo & WeaverQinfo_RESIZE_LOCK) {
			CPU_YIELD;
			mqinfo = atomic_load_explicit(&this->mqinfo, memory_order_relaxed);
		}

		goto acquire_reference;
	}

	has_reference:;
	WeaverQueue *queue = this->queue[mqinfo & WeaverQinfo_INDEX_BIT];
	const usize pos = WeaverQinfo_pos(mqinfo);
	usize capacity = queue->capacity;

	// check if the queue needs to be resized before
	// inserting our tasks
	if (capacity < (pos + tasks_size)) {
		// try to acquire resize lock
		mqinfo = atomic_fetch_or_explicit(
			&this->mqinfo, WeaverQinfo_RESIZE_LOCK, memory_order_acquire
		);

		// another thread started to resize first
		if (mqinfo & WeaverQinfo_RESIZE_LOCK)
			goto yield_to_resize_lock;

		// wait for other threads to release their references
		while ((mqinfo & WeaverQinfo_RC_MASK) != WeaverQinfo_RC_ONE) {
			CPU_YIELD;
			mqinfo = atomic_load_explicit(&this->mqinfo, memory_order_acquire);
		}

		// perform resize

		capacity = WeaverQueue_nextcapacity(capacity, pos + tasks_size);

		queue = Allocator_resize(
			this->queue_alc, queue, ZZWeaverQueue_allocsize(capacity)
		);
		queue->capacity = capacity;

		// update queue pointer in runtime
		this->queue[mqinfo & WeaverQinfo_INDEX_BIT] = queue;

		// release lock
		atomic_fetch_and_explicit(&this->mqinfo, ~WeaverQinfo_RESIZE_LOCK, memory_order_release);

		goto has_reference;
	}

	// make sure the position is the same as when we checked for capacity
	if (
		!atomic_compare_exchange_strong_explicit(
			&this->mqinfo, &mqinfo, mqinfo + (tasks_size * WeaverQinfo_POS_ONE),
			memory_order_acquire, memory_order_acquire
		)
	)
		goto has_reference;

	// write tasks
	memcpy(&queue->tasks[pos], tasks, sizeof(AsyncTask) * tasks_size);

	// release reference
	mqinfo = atomic_fetch_sub_explicit(
		&this->mqinfo, WeaverQinfo_RC_ONE, memory_order_release
	);

	// if this wasn't the last mqinfo reference let another thread do a swap
	if ((mqinfo & WeaverQinfo_RC_MASK) != WeaverQinfo_RC_ONE)
		return;

	// else this was the last reference, check if a swap is needed

	// acquire qinfo reference
	WeaverQinfo qinfo = atomic_fetch_add_explicit(
		&this->qinfo, WeaverQinfo_RC_ONE, memory_order_acquire
	);
	if (qinfo & WeaverQinfo_SWAP_LOCK) {
		WeaverQinfo qinfo_0 = qinfo;
		// swap is already being performed by other thread, we need to release our reference
		while (true) {
			if (
				atomic_compare_exchange_weak_explicit(
					&this->qinfo, &qinfo, qinfo - WeaverQinfo_RC_ONE,
					memory_order_acquire, memory_order_acquire
				)
			)
				break;

			if ((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)
				break;
		}

		return;
	}

	// if queue is not depleted, we don't need to swap
	if (WeaverQinfo_pos(qinfo) < this->queue[qinfo & WeaverQinfo_INDEX_BIT]->size) {
		// release reference
		atomic_fetch_sub_explicit(&this->qinfo, WeaverQinfo_RC_ONE, memory_order_relaxed);
		return;
	}

	// perform swap
	qinfo = atomic_fetch_sub_explicit(&this->qinfo, WeaverQinfo_RC_ONE, memory_order_acquire);
	Weaver_swap(this, qinfo);
}

#include "WeaverThread.c"
