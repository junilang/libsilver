AsyncTask WeaverThread_runtask(
	WeaverThread const *this, Weaver *const rt, AsyncTask task
) {
	AsyncResult result;
	AsyncIntent intent = AsyncTask_call(task, (AsyncRT*)rt, &result);

	switch (intent) {
		case AsyncIntent_CALL:
			#if Weaver_CALL_IMMEDIATE
				return result.task;
			#endif

		case AsyncIntent_RESUME:
			#if Weaver_RESUME_IMMEDIATE
				return result.task;
			#endif

		case AsyncIntent_SUSPEND:
			Weaver_submit(rt, &result.task, 1);
			FALLTHROUGH;

		case AsyncIntent_YIELD:
			return AsyncTask_NULL;
	}

	UNREACHABLE;
}

Ptr WeaverThread_main(Ptr vthis) {
	WeaverThread *this = vthis;
	Weaver *rt;
	AsyncTask pending_task = AsyncTask_NULL;

	uint state = atomic_exchange_explicit(&this->state, WeaverThread_IDLE, memory_order_acquire);
	#if Weaver_SAFE
		if (state < WeaverThread_BOOT)
			PANIC("invalid boot state");
	#endif

	#if Weaver_DEBUG
		t_WeaverThread_id = (state - WeaverThreadState_BOOT) + 1;
	#endif

	// calculate pointer to runtime based on our thread id
	rt = (Weaver*)((ubyte*)this - (
		offsetof(Weaver, threads) +
		(state - WeaverThread_BOOT) * sizeof(WeaverThread)
	));

	goto idle; // boot sequence complete

	load_state:;
	state = atomic_load_explicit(&this->state, memory_order_acquire);

	dispatch_state:;
	switch (state) {
		case WeaverThread_DOWN: goto Bdown;
		case WeaverThread_IDLE: goto Bidle;
		case WeaverThread_RUN: goto Brun;
	}

	UNREACHABLE;

	if (0) Brun: {
		if (AsyncTask_isnull(pending_task)) goto fetch_task;

		AsyncTaskIO task_io;
		AsyncIntent intent = AsyncTask_call(pending_task, 0, (AsyncRT*)rt, &task_io);
		switch (intent) {
			case AsyncIntent_CALL:
				#if Weaver_CALL_IMMEDIATE
					goto immediate;
				#else
					goto suspend_task;
				#endif
			case AsyncIntent_RESUME:
				#if Weaver_RESUME_IMMEDIATE
					goto immediate;
				#else
					goto suspend_task;
				#endif
			case AsyncIntent_SUSPEND:
				goto suspend_task;
			case AsyncIntent_YIELD:
				goto load_state;
			default:
		}

		UNREACHABLE;

		if (0) immediate: {
			pending_task = task_io.out_task;
			#if Weaver_IMMEDIATE_NOSYNC
				goto Brun;
			#else
				goto load_state;
			#endif
		}

		if (0) suspend_task: {
			Weaver_submit(rt, &task_io.out_task, 1);
			goto load_state;
		}

	}

	if (0) fetch_task: {
		// acquire read reference
		WeaverQinfo qinfo = atomic_fetch_add_explicit(
			&rt->qinfo, WeaverQinfo_RC_ONE, memory_order_acquire
		);

		if (qinfo & WeaverQinfo_SWAP_LOCK) {
			WeaverQinfo qinfo_0 = qinfo;
			// a swap is being performed by another thread
			while (true) {
				if (
					atomic_compare_exchange_weak_explicit(
						&rt->qinfo, &qinfo, qinfo - WeaverQinfo_RC_ONE,
						memory_order_acquire, memory_order_relaxed
					)
				)
					break;

				if ((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)
					goto load_state;
			}

			// wait for swap to complete
			while (!((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)) {
				CPU_YIELD;
				qinfo = atomic_load_explicit(&rt->qinfo, memory_order_acquire);
			}

			goto load_state;
		}

		// we have a valid reference and can read a task from the queue
		qinfo = atomic_fetch_add_explicit(&rt->qinfo, WeaverQinfo_POS_ONE, memory_order_acquire);

		const usize pos = WeaverQinfo_pos(qinfo);
		const WeaverQueue *queue = rt->queue[qinfo & WeaverQinfo_INDEX_BIT];

		// check if queue is empty
		if (pos < queue->size) {
			pending_task = queue->tasks[pos];
			atomic_fetch_sub_explicit(&rt->qinfo, WeaverQinfo_RC_ONE, memory_order_relaxed);
			goto load_state;
		}

		// queue is empty, try to swap
		// release our reference
		qinfo = atomic_fetch_sub_explicit(&rt->qinfo, WeaverQinfo_RC_ONE, memory_order_acq_rel);

		// try to swap if mqueue is not empty
		if (atomic_load_explicit(&rt->mqinfo, memory_order_acquire) & WeaverQinfo_POS_MASK) {
			if (!Weaver_swap(rt, qinfo)) {
				WeaverQinfo qinfo_0 = qinfo;
				// wait for other thread to complete swap
				do {
					CPU_YIELD;
					qinfo = atomic_load_explicit(&rt->qinfo, memory_order_acquire);
				} while (!((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK));
			}

			goto load_state;
		}

		// else if mqueue is empty try going idle

		if (
			atomic_compare_exchange_strong_explicit(
				&this->state, &state, WeaverThread_IDLE,
				memory_order_acq_rel, memory_order_acquire
			)
		)
			goto idle;
		else
			goto dispatch_state;

	}


	if (0) Bidle: {
		// resubmit task if thread was requested to go idle
		if (!AsyncTask_isnull(pending_task)) {
			Weaver_submit(rt, &pending_task, 1);
			pending_task = AsyncTask_NULL;
		}

		idle:;

		if (atomic_fetch_sub_explicit(&rt->threads_sync, 1, memory_order_acq_rel) == 1) {
			uint lock = Weaver_LOCK_JOIN;
			if (
				atomic_compare_exchange_strong_explicit(
					&rt->lock, &lock, Weaver_LOCK_NONE,
					memory_order_acq_rel, memory_order_acquire
				)
			) {
				umtx_wake_all(&rt->lock);
			}
		}

		do {
			switch (umtx_wait(&this->state, WeaverThread_IDLE)) {
				case umtxResult_MISMATCH:
					goto idle_mismatch;
				default:;
			}
		} while (atomic_load_explicit(&this->state, memory_order_acquire) == WeaverThread_IDLE);

		idle_mismatch:;
		atomic_fetch_add_explicit(&rt->threads_sync, 1, memory_order_acq_rel);

		goto load_state;
	}

	if (0) Bdown: {
		this->orphaned_task = pending_task;

		if (atomic_load_explicit(&rt->threads_sync, memory_order_acquire) == rt->threads_size) {
			uint lock = Weaver_LOCK_DOWN;
			if (
				atomic_compare_exchange_strong_explicit(
					&rt->lock, &lock, Weaver_LOCK_NONE,
					memory_order_acq_rel, memory_order_relaxed
				)
			) {
				umtx_wake_all(&rt->lock);
			}
		}

		return nullptr;
	}

	UNREACHABLE;
}
