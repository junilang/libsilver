#ifndef Weaver_SAFE
	#define Weaver_SAFE BUILD_SAFE
#endif

#ifndef Weaver_DEBUG
	#define Weaver_DEBUG BUILD_DEBUG
#endif

#ifndef Weaver_CALL_ENQUEUE
	#define Weaver_CALL_ENQUEUE true
#endif

#ifndef Weaver_RESUME_ENQUEUE
	#define Weaver_RESUME_ENQUEUE true
#endif

#if Weaver_DEBUG
	_Thread_local u32 t_WeaverThread_id = 0;

	#define Weaver_DBG(...) FPRINT_ATOMIC( \
		stderr, "[",t_WeaverThread_id,"] ",__func__,": "__VA_OPT__(,)__VA_ARGS__,"\n" \
	)
#else
	#define Weaver_DBG(...)
#endif

// TODO use explicit memory ordering on atomic operations

#include "WeaverQueue.h"
#include "WeaverThread.h"
#include "Weaver.h"

Ptr WeaverThread_main(Ptr);

void WeaverThread_boot(Weaver *rt, u16 id) {
	WeaverThread *this = &rt->threads[id];
	atomic_init(&this->state, WeaverThreadState_BOOT + id);
	pthread_create(&this->thread, nullptr, &WeaverThread_main, this);
}

void Weaver_boot(Weaver *this) {
	const u16 size = this->threads_size;

	atomic_store(&this->lock, Weaver_LOCK_JOIN);
	atomic_store(&this->threads_sync, size);

	for (u16 id = 0; id < size; id++) {
		WeaverThread_boot(this, id);
	}

	while (atomic_load(&this->lock) == Weaver_LOCK_JOIN) {
		syscall(SYS_futex, &this->lock, FUTEX_WAIT,
			Weaver_LOCK_JOIN, nullptr
		);
	}

	atomic_store(&this->lock, Weaver_LOCK_NONE);

	#if Weaver_SAFE
		if (atomic_load(&this->threads_sync) != 0)
			PANIC("boot thread sync failed");
	#endif
}

void Weaver_wake(Weaver *this, usize n) {
	if (atomic_load(&this->lock) != Weaver_LOCK_NONE)
		return;

	auto it = this->threads;
	auto const end = it + this->threads_size;

	for (; n && (it < end); it++) {
		uint state = WeaverThreadState_IDLE;
		if (atomic_compare_exchange_strong(&it->state, &state, WeaverThreadState_RUN)) {
			syscall(SYS_futex, &it->state, FUTEX_WAKE, INT_MAX);
			n--;
		}
	}
}

void Weaver_join(Weaver *this) {
	uint lock = Weaver_LOCK_NONE;

	if (
		!atomic_compare_exchange_strong(
			&this->lock, &lock, Weaver_LOCK_JOIN
		)
	)
		PANIC("invalid lock state");

	// wait for all threads to be idle
	auto const sync = atomic_load(&this->threads_sync);
	if (sync == 0) {
		lock = Weaver_LOCK_JOIN;
		if (
			!atomic_compare_exchange_strong(
				&this->lock, &lock, Weaver_LOCK_NONE
			)
		)
			PANIC("invalid lock state");

		goto down;
	}

	while (atomic_load(&this->lock) == Weaver_LOCK_JOIN) {
		syscall(SYS_futex, &this->lock, FUTEX_WAIT,
			Weaver_LOCK_JOIN, nullptr
		);
	}

	down:;
	lock = Weaver_LOCK_NONE;
	if (
		!atomic_compare_exchange_strong(
			&this->lock, &lock, Weaver_LOCK_DOWN
		)
	)
		PANIC("invalid lock state");

	// wake up all threads and set their state to down
	auto it = this->threads;
	auto const end = it + this->threads_size;
	for (; it < end; it++) {
		if (atomic_exchange(&it->state, WeaverThreadState_DOWN) == WeaverThreadState_IDLE) {
			syscall(SYS_futex, &it->state, FUTEX_WAKE, INT_MAX);
		}
	}

	// wait for all threads to shut down
	while (atomic_load(&this->lock) == Weaver_LOCK_DOWN) {
		syscall(SYS_futex, &this->lock, FUTEX_WAIT,
			Weaver_LOCK_DOWN, nullptr
		);
	}

	// join threads using pthreads interface
	it = this->threads;
	for (; it < end; it++) {
		pthread_join(it->thread, nullptr);
	}
}

// returns true if the calling thread successfully performed a swap
bool Weaver_swap(Weaver *this, WeaverQinfo expected_index) {
	WeaverQinfo qinfo = atomic_fetch_or(&this->qinfo, WeaverQinfo_SWAP_LOCK);

	if (qinfo & WeaverQinfo_SWAP_LOCK) {
		// another thread started swapping before us
		return false;
	}

	// if the expected index provided by caller does not match the current index
	// a swap operation already must have happened between when the caller
	// decided a swap was necessary and we acquired our lock
	if ((qinfo ^ expected_index) & WeaverQinfo_INDEX_MASK) {
		// release lock
		atomic_fetch_and(&this->qinfo, ~WeaverQinfo_SWAP_LOCK);
		return true;
	}

	// acquire mqinfo lock
	WeaverQinfo mqinfo = atomic_fetch_or(&this->mqinfo, WeaverQinfo_SWAP_LOCK);

	// wait until other threads release references
	while (mqinfo & (WeaverQinfo_RC_MASK | WeaverQinfo_RESIZE_LOCK)) {
		Spinlock_PAUSE;
		mqinfo = atomic_load(&this->mqinfo);
	}

	// qinfo doesn't use the resize lock
	while (qinfo & WeaverQinfo_RC_MASK) {
		Spinlock_PAUSE;
		qinfo = atomic_load(&this->qinfo);
	}

	// we now have both locks and no valid references are held by other threads

	// set queue size
	const usize size = WeaverQinfo_pos(mqinfo);
	this->queue[mqinfo & WeaverQinfo_INDEX_BIT]->size = size;

	// perform swap
	atomic_store(&this->mqinfo, (mqinfo + 1) & WeaverQinfo_INDEX_MASK);
	atomic_store(&this->qinfo, (qinfo + 1) & WeaverQinfo_INDEX_MASK);

	// wake threads
	Weaver_wake(this, size);
	return true;
}

void Weaver_submit(Weaver *this, const AsyncTask *tasks, usize tasks_size) {
	acquire_reference:;
	WeaverQinfo mqinfo = atomic_fetch_add(&this->mqinfo, WeaverQinfo_RC_ONE);

	// if another thread is performing a swap
	if (mqinfo & WeaverQinfo_SWAP_LOCK) {
		WeaverQinfo mqinfo_0 = mqinfo;
		while (true) {
			// try to release our reference
			if (
				atomic_compare_exchange_strong(
					&this->mqinfo, &mqinfo, mqinfo - WeaverQinfo_RC_ONE
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
			Spinlock_PAUSE;
			mqinfo = atomic_load(&this->mqinfo);
		}

		goto acquire_reference;
	}

	if (mqinfo & WeaverQinfo_RESIZE_LOCK) {
		yield_to_resize_lock:;

		while (true) {
			if (
				atomic_compare_exchange_strong(
					&this->mqinfo, &mqinfo, mqinfo - WeaverQinfo_RC_ONE
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
			Spinlock_PAUSE;
			mqinfo = atomic_load(&this->mqinfo);
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
		mqinfo = atomic_fetch_or(&this->mqinfo, WeaverQinfo_RESIZE_LOCK);

		// another thread started to resize first
		if (mqinfo & WeaverQinfo_RESIZE_LOCK)
			goto yield_to_resize_lock;

		// wait for other threads to release their references
		while ((mqinfo & WeaverQinfo_RC_MASK) != WeaverQinfo_RC_ONE) {
			Spinlock_PAUSE;
			mqinfo = atomic_load(&this->mqinfo);
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
		atomic_fetch_and(&this->mqinfo, ~WeaverQinfo_RESIZE_LOCK);

		goto has_reference;
	}

	// make sure the position is the same as when we checked for capacity
	if (
		!atomic_compare_exchange_strong(
			&this->mqinfo, &mqinfo, mqinfo + (tasks_size * WeaverQinfo_POS_ONE)
		)
	)
		goto has_reference;

	// write tasks
	memcpy(&queue->tasks[pos], tasks, sizeof(AsyncTask) * tasks_size);

	// release reference
	mqinfo = atomic_fetch_sub(&this->mqinfo, WeaverQinfo_RC_ONE);

	// if this wasn't the last mqinfo reference let another thread do a swap
	if ((mqinfo & WeaverQinfo_RC_MASK) != WeaverQinfo_RC_ONE)
		return;

	// else this was the last reference, check if a swap is needed

	// acquire qinfo reference
	WeaverQinfo qinfo = atomic_fetch_add(&this->qinfo, WeaverQinfo_RC_ONE);
	if (qinfo & WeaverQinfo_SWAP_LOCK) {
		WeaverQinfo qinfo_0 = qinfo;
		// swap is already being performed by other thread, we need to release our reference
		while (true) {
			if (
				atomic_compare_exchange_strong(
					&this->qinfo, &qinfo, qinfo - WeaverQinfo_RC_ONE
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
		atomic_fetch_sub(&this->qinfo, WeaverQinfo_RC_ONE);
		return;
	}

	// perform swap
	qinfo = atomic_fetch_sub(&this->qinfo, WeaverQinfo_RC_ONE);
	Weaver_swap(this, qinfo);
}

#include "WeaverThread.c"
