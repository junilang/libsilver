AsyncTask WeaverThread_runtask(
	WeaverThread const *this, Weaver *const rt, AsyncTask task
) {
	auto const task_data = AsyncTask_data(task);
	auto result = task_data->entry(Weaver_upcast(rt), task);

	switch (result.intent) {
		case AsyncIntent_SUSPEND: return AsyncTask_NULL;
		case AsyncIntent_FINISH: goto Bfinish;
		case AsyncIntent_PANIC: goto Bpanic;
		default: goto Byield;
	}

	if (0) Bfinish: {
		auto const metadata = AsyncTask_metadata(task);

		if (metadata & FLAG(AsyncTaskFlag, RESUMING)) {
			task = task_data->resume;

			#if Weaver_SAFE
				if (AsyncTask_isnull(task)) return AsyncTask_NULL;
			#endif

			goto resume;
		} else {
			auto future = task_data->future;
			if (AsyncFuture_isnull(future)) return AsyncTask_NULL;

			result = AsyncFuture_callback(future);
			switch (result.intent) {
				case AsyncIntent_FINISH:
					#if Weaver_SAFE
						PANIC("future shouldn't return finish intent");
					#endif
				case AsyncIntent_SUSPEND: return AsyncTask_NULL;
				case AsyncIntent_PANIC: goto Bpanic;
				default:;
			}

			task = result.next;
			goto resume;
		}
	}

	if (0) resume: {
		if (
			Weaver_RESUME_ENQUEUE
			|| (AsyncTask_metadata(task) & FLAG(AsyncTaskFlag, SUSPEND))
		) {
			Weaver_submit(rt, &task, 1);
			return AsyncTask_NULL;
		} else {
			return task;
		}
	}

	if (0) Byield: {
		if (
			Weaver_YIELD_ENQUEUE ||
			(AsyncTask_metadata(result.next) & FLAG(AsyncTaskFlag, SUSPEND))
		) {
			Weaver_submit(rt, &result.next, 1);
			return AsyncTask_NULL;
		} else {
			return result.next;
		}
	}

	if (0) Bpanic: {
		PANIC("task panicked");
	}

	UNREACHABLE;
}

Ptr WeaverThread_main(Ptr vthis) {
	WeaverThread *this = vthis;
	Weaver *rt;
	AsyncTask pending_task = AsyncTask_NULL;


	uint state = atomic_exchange(&this->state, WeaverThreadState_IDLE);
	#if Weaver_SAFE
		if (state < WeaverThreadState_BOOT)
			PANIC("invalid boot state");
	#endif

	rt = (Weaver*)((ubyte*)this - (
		offsetof(Weaver, threads) +
		(state - WeaverThreadState_BOOT) * sizeof(WeaverThread)
	));

	goto idle;

	// boot sequence complete

	load_state:;
	state = atomic_load(&this->state);

	dispatch_state:;
	switch (state) {
		default: UNREACHABLE;
		case WeaverThreadState_DOWN: goto Bdown;
		case WeaverThreadState_IDLE: goto Bidle;
		case WeaverThreadState_RUN: goto Brun;
	}

	if (0) Brun: {
		if (AsyncTask_isnull(pending_task)) goto fetch_task;

		pending_task = WeaverThread_runtask(this, rt, pending_task);
		goto load_state;
	}

	if (0) fetch_task: {
		WeaverQinfo qinfo = atomic_fetch_add(&rt->qinfo,
			WeaverQinfo_POS_ONE | WeaverQinfo_RC_ONE
		);

		// a swap is being performed
		if (WeaverQinfo_lock(qinfo)) {
			WeaverQinfo qinfo_0 = qinfo;
			while (true) {
				// try to release our reference if the swap is still underway
				if (
					atomic_compare_exchange_strong(
						&rt->qinfo, &qinfo, qinfo - WeaverQinfo_RC_ONE
					)
				)
					goto wait_for_swap;

				// if index changed, the swap was successfully performed
				if ((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)
					goto load_state;
			}
		}

		auto const qpos = WeaverQinfo_pos(qinfo);
		const WeaverQueue *queue = rt->queue[qinfo & WeaverQinfo_INDEX_BIT];

		#if 0 && Weaver_DEBUG
			FPRINT(stderr, "queue=",(Ptr)queue,"\n");
		#endif

		auto const qsize = queue->size;

		if (qpos < qsize) {
			pending_task = queue->tasks[qpos];
		}

		if ((qpos + 1) >= qsize) goto perform_swap;

		atomic_fetch_sub(&rt->qinfo, WeaverQinfo_RC_ONE);

		goto load_state;

		if (0) perform_swap: {
			qinfo = atomic_fetch_or(&rt->qinfo, WeaverQinfo_LOCK_BIT);

			// queue is already being swapped
			if (WeaverQinfo_lock(qinfo)) {
				// we inremented the refcounter before the other thread started performing the swap
				// thus we need to release it
				atomic_fetch_sub(&rt->qinfo, WeaverQinfo_RC_ONE);

				if (AsyncTask_isnull(pending_task))
					goto wait_for_swap;
				else
					goto load_state;
			}

			WeaverQinfo mqinfo = atomic_fetch_or(&rt->mqinfo, WeaverQinfo_LOCK_BIT);
			while (WeaverQinfo_lock(mqinfo)) {
				// another thread has the lock
				// since we have the lock for qinfo, the other thread cannot be swapping
				// wait until the lock is released

				Spinlock_PAUSE;
				mqinfo = atomic_fetch_or(&rt->mqinfo, WeaverQinfo_LOCK_BIT);
			}

			// we have both locks

			// wait for mqinfo to be free first
			// this allows a submitting thread to assume that
			// it is safe to acquire and release references to qinfo
			// atomically as long as it has a valid reference to mqinfo

			// we don't hold a reference so check for 0
			while (WeaverQinfo_maskrc(mqinfo)) {
				Spinlock_PAUSE;
				mqinfo = atomic_load(&rt->mqinfo);
			}

			// check for 1 reference because we hold one
			while (WeaverQinfo_maskrc(qinfo) != WeaverQinfo_RC_ONE) {
				Spinlock_PAUSE;
				qinfo = atomic_load(&rt->qinfo);
			}

			// if mqueue is empty idle
			if (!WeaverQinfo_maskpos(mqinfo)) {
				if (AsyncTask_isnull(pending_task)) {
					bool state_changed = !atomic_compare_exchange_strong(
						&this->state, &state, WeaverThreadState_IDLE
					);

					// unlock mqueue
					atomic_fetch_and(&rt->mqinfo, ~WeaverQinfo_LOCK_BIT);
					// release qinfo reference
					atomic_fetch_sub(&rt->qinfo, WeaverQinfo_RC_ONE);
					// unlock qinfo
					atomic_fetch_and(&rt->qinfo, ~WeaverQinfo_LOCK_BIT);

					if (state_changed) {
						goto dispatch_state;
					} else {
						goto Bidle;
					}
				} else {
					atomic_fetch_and(&rt->mqinfo, ~WeaverQinfo_LOCK_BIT);
					atomic_fetch_sub(&rt->qinfo, WeaverQinfo_RC_ONE);
					atomic_fetch_and(&rt->qinfo, ~WeaverQinfo_LOCK_BIT);
					goto load_state;
				}
			}

			// perform swap
			ZZWeaver_swap(rt, qinfo, mqinfo);

			goto load_state;
		}

		if (0) wait_for_swap: {
			// other thread started swap before we incremented the refcount
			// so we don't need to decrement it becuase it will be reset on swap

			WeaverQinfo qinfo_0 = qinfo;

			while (true) {
				qinfo = atomic_load(&rt->qinfo);

				// if index changed swap has completed
				if ((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)
					goto load_state;

				Spinlock_PAUSE;
			}
		}

	}


	if (0) Bidle: {
		if (!AsyncTask_isnull(pending_task)) {
			Weaver_submit(rt, &pending_task, 1);
			pending_task = AsyncTask_NULL;
		}

		idle:;

		if (atomic_fetch_sub(&rt->threads_sync, 1) == 1) {
			uint lock = Weaver_LOCK_JOIN;
			if (atomic_compare_exchange_strong(&rt->lock, &lock, Weaver_LOCK_NONE)) {
				syscall(SYS_futex, &rt->lock, FUTEX_WAKE, INT_MAX);
			}
		}

		while (atomic_load(&this->state) == WeaverThreadState_IDLE) {
			syscall(SYS_futex, &this->state, FUTEX_WAIT,
				WeaverThreadState_IDLE, nullptr
			);
		}

		atomic_fetch_add(&rt->threads_sync, 1);

		goto load_state;
	}

	if (0) Bdown: {
		this->orphaned_task = pending_task;

		if (atomic_fetch_sub(&rt->threads_sync, 1) == 1) {
			uint lock = Weaver_LOCK_DOWN;
			if (atomic_compare_exchange_strong(&rt->lock, &lock, Weaver_LOCK_NONE)) {
				syscall(SYS_futex, &rt->lock, FUTEX_WAKE, INT_MAX);
			}
		}

		return nullptr;
	}

	UNREACHABLE;
}
