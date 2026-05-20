#ifndef Weaver_SAFE
	#define Weaver_SAFE BUILD_SAFE
#endif

#ifndef Weaver_DEBUG
	#define Weaver_DEBUG BUILD_DEBUG
#endif

#ifndef Weaver_YIELD_ENQUEUE
	#define Weaver_YIELD_ENQUEUE true
#endif

#ifndef Weaver_RESUME_ENQUEUE
	#define Weaver_RESUME_ENQUEUE true
#endif

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
	if (this->threads_size < n) {
		n = this->threads_size;
	}

	auto it = this->threads;
	auto const end = it + n;

	for (; it < end; it++) {
		uint state = WeaverThreadState_IDLE;
		if (atomic_compare_exchange_strong(&it->state, &state, WeaverThreadState_RUN)) {
			syscall(SYS_futex, &it->state, FUTEX_WAKE, INT_MAX);
		}
	}
}

void Weaver_join(Weaver *this) {
	atomic_store(&this->lock, Weaver_LOCK_JOIN);

	if (atomic_load(&this->threads_sync) == 0) goto down;

	while (atomic_load(&this->lock) == Weaver_LOCK_JOIN) {
		syscall(SYS_futex, &this->lock, FUTEX_WAIT,
			Weaver_LOCK_JOIN, nullptr
		);
	}

	down:;

	atomic_store(&this->lock, Weaver_LOCK_DOWN);

	auto it = this->threads;
	auto const end = it + this->threads_size;
	for (; it < end; it++) {
		uint state = WeaverThreadState_IDLE;
		if (atomic_compare_exchange_strong(&it->state, &state, WeaverThreadState_DOWN)) {
			syscall(SYS_futex, &it->state, FUTEX_WAKE, INT_MAX);
		}
	}

	while (atomic_load(&this->lock) == Weaver_LOCK_DOWN) {
		syscall(SYS_futex, &this->lock, FUTEX_WAIT,
			Weaver_LOCK_DOWN, nullptr
		);
	}

	it = this->threads;
	for (; it < end; it++) {
		pthread_join(it->thread, nullptr);
	}
}

void ZZWeaver_swap(Weaver *this, WeaverQinfo qinfo, WeaverQinfo mqinfo) {
	auto pos = WeaverQinfo_pos(mqinfo);

	this->queue[mqinfo & WeaverQinfo_INDEX_BIT]->size = pos;

	atomic_store(&this->qinfo, (qinfo + 1) & WeaverQinfo_INDEX_MASK);
	atomic_store(&this->mqinfo, (mqinfo + 1) & WeaverQinfo_INDEX_MASK);

	Weaver_wake(this, WeaverQinfo_pos(mqinfo));
}

void Weaver_submit(Weaver *this, const AsyncTask *tasks, usize tasks_size) {
	#if Weaver_SAFE
		if (tasks_size > WeaverQinfo_POS_MAX)
			PANIC("tasks_size too large");
	#endif

	const WeaverQinfo infomod = (tasks_size * WeaverQinfo_POS_ONE) | WeaverQinfo_RC_ONE;

	WeaverQinfo mqinfo;
	WeaverQinfo mqinfo_0;

	goto resubmit;

	wait_for_lock: {
		while (true) {
			mqinfo = atomic_load(&this->mqinfo);

			if ((mqinfo ^ mqinfo_0) & WeaverQinfo_INDEX_MASK)
				goto resubmit;

			if (!WeaverQinfo_lock(mqinfo))
				break;

			Spinlock_PAUSE;
		}

		// acquire reference
		resubmit:;
		mqinfo = atomic_fetch_add(&this->mqinfo, infomod);
	}

	if (WeaverQinfo_lock(mqinfo)) {
		mqinfo_0 = mqinfo;

		while (true) {
			// try to release reference in case other thread is waiting
			if (
				atomic_compare_exchange_strong(
					&this->mqinfo, &mqinfo, mqinfo - infomod
				)
			)
				goto wait_for_lock;

			// other thraed was performing swap, need to resubmit
			if ((mqinfo ^ mqinfo_0) & WeaverQinfo_INDEX_MASK)
				goto resubmit;

			// other thread unlocked, we still have our reference
			if (!WeaverQinfo_lock(mqinfo))
				break;
		}
	}

	// we have acquired a valid reference to mqinfo

	const usize pos = WeaverQinfo_pos(mqinfo);
	WeaverQueue *queue = this->queue[mqinfo & WeaverQinfo_INDEX_BIT];
	usize capacity = queue->capacity;

	WeaverQinfo qinfo;

	if (capacity < (pos + tasks_size)) {
		// acquire lock
		mqinfo = atomic_fetch_or(&this->mqinfo, WeaverQinfo_LOCK_BIT);

		// if other thread acquired the lock first
		if (WeaverQinfo_lock(mqinfo)) {
			// other thread must be waiting for us to release reference
			mqinfo = atomic_fetch_sub(&this->mqinfo, WeaverQinfo_RC_ONE);

			while (true) {
				// swap was performed
				if ((mqinfo ^ mqinfo_0) & WeaverQinfo_INDEX_MASK)
					goto resubmit;

				if (!WeaverQinfo_lock(mqinfo)) break;

				Spinlock_PAUSE;
				mqinfo = atomic_fetch_or(&this->mqinfo, WeaverQinfo_LOCK_BIT);
			}

			// we have acquired the lock and can safely recquire reference
			mqinfo = atomic_fetch_add(&this->mqinfo, WeaverQinfo_RC_ONE);
		}

		// wait for other threads to release their references
		while (WeaverQinfo_maskrc(mqinfo) == WeaverQinfo_RC_ONE) {
			Spinlock_PAUSE;
			mqinfo = atomic_load(&this->mqinfo);
		}

		// perform resize
		capacity = WeaverQueue_nextcapacity(capacity, WeaverQinfo_pos(mqinfo));

		queue = Allocator_resize(
			this->queue_alc, queue, ZZWeaverQueue_allocsize(capacity)
		);

		this->queue[WeaverQinfo_index(mqinfo_0)] = queue;

		memcpy(&queue->tasks[pos], tasks, sizeof(AsyncTask) * tasks_size);

		//	if queue was empty before, a swap is going to be needed
		// worker threads might be asleep and cannot perform the swap themselves
		if (pos != 0) {
			// release lock and reference
			atomic_fetch_sub(&this->mqinfo, WeaverQinfo_RC_ONE);
			atomic_fetch_and(&this->mqinfo, ~WeaverQinfo_LOCK_BIT);
			return;
		}

		// check if swap is needed
		qinfo = atomic_fetch_add(&this->qinfo, WeaverQinfo_RC_ONE);

		if (
			// swap is already in progress by other thread
			WeaverQinfo_lock(qinfo)

			// queue is not depleted
			|| (
				WeaverQinfo_pos(qinfo) <
				this->queue[qinfo & WeaverQinfo_INDEX_BIT]->size
			)

			// try to acquire qinfo lock
			// if it was already locked release and let the other perform swap
			|| WeaverQinfo_lock((qinfo = atomic_fetch_or(&this->qinfo, WeaverQinfo_LOCK_BIT)))
		) {
			// release and return
			atomic_fetch_sub(&this->qinfo, WeaverQinfo_RC_ONE);
			atomic_fetch_sub(&this->mqinfo, WeaverQinfo_RC_ONE);
			atomic_fetch_and(&this->mqinfo, ~WeaverQinfo_LOCK_BIT);
			return;
		}

	} else {

		memcpy(&queue->tasks[pos], tasks, sizeof(AsyncTask) * tasks_size);

		if (pos != 0) {
			atomic_fetch_sub(&this->mqinfo, WeaverQinfo_RC_ONE);
			return;
		}

		qinfo = atomic_fetch_add(&this->qinfo, WeaverQinfo_RC_ONE);

		if (
			// swap is already in progress by other thread
			WeaverQinfo_lock(qinfo)

			// queue is not depleted
			|| (
				WeaverQinfo_pos(qinfo) <
				this->queue[qinfo & WeaverQinfo_INDEX_BIT]->size
			)

			// try to acquire qinfo lock
			// if it was already locked release and let the other perform swap
			|| WeaverQinfo_lock((qinfo = atomic_fetch_or(&this->qinfo, WeaverQinfo_LOCK_BIT)))
		) {
			atomic_fetch_sub(&this->qinfo, WeaverQinfo_RC_ONE);
			atomic_fetch_sub(&this->mqinfo, WeaverQinfo_RC_ONE);
			return;
		}

		// acquire mqinfo lock

		mqinfo = atomic_fetch_or(&this->mqinfo, WeaverQinfo_LOCK_BIT);
		if (WeaverQinfo_lock(mqinfo)) {
			// lock was already taken

			do {
				Spinlock_PAUSE;
				mqinfo = atomic_fetch_or(&this->mqinfo, WeaverQinfo_LOCK_BIT);

				// no need to check for swaps because we have the qinfo lock
			} while (WeaverQinfo_lock(mqinfo));
		}
	}

	// wait until all other references are released
	while (WeaverQinfo_maskrc(mqinfo) != WeaverQinfo_RC_ONE) {
		Spinlock_PAUSE;
		mqinfo = atomic_load(&this->mqinfo);
	}

	while (WeaverQinfo_maskrc(qinfo) != WeaverQinfo_RC_ONE) {
		Spinlock_PAUSE;
		qinfo = atomic_load(&this->qinfo);
	}

	// both locks are acquired, perform swap
	ZZWeaver_swap(this, qinfo, mqinfo);
}

#include "WeaverThread.c"
