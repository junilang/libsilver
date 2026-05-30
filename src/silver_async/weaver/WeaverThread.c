AsyncTask WeaverThread_runtask(
	WeaverThread const *this, Weaver *const rt, AsyncTask task
) {
	auto const task_data = AsyncTask_data(task);

	//Weaver_DBG("task=",(Ptr)task_data);

	auto result = task_data->entry(Weaver_upcast(rt), task);

	switch (result.intent) {
		case AsyncIntent_YIELD: return AsyncTask_NULL;
		case AsyncIntent_FINISH: goto Bfinish;
		default: goto Bnext;
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
				case AsyncIntent_YIELD:
				case AsyncIntent_FINISH:
					return AsyncTask_NULL;

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

	if (0) Bnext: {
		if (
			Weaver_CALL_ENQUEUE ||
			(AsyncTask_metadata(result.next) & FLAG(AsyncTaskFlag, SUSPEND))
		) {
			Weaver_submit(rt, &result.next, 1);
			return AsyncTask_NULL;
		} else {
			return result.next;
		}
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

	#if Weaver_DEBUG
		t_WeaverThread_id = (state - WeaverThreadState_BOOT) + 1;
	#endif

	// calculate pointer to runtime based on our thread id
	rt = (Weaver*)((ubyte*)this - (
		offsetof(Weaver, threads) +
		(state - WeaverThreadState_BOOT) * sizeof(WeaverThread)
	));

	goto idle; // boot sequence complete

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
		// acquire read reference
		WeaverQinfo qinfo = atomic_fetch_add(&rt->qinfo, WeaverQinfo_RC_ONE);

		if (qinfo & WeaverQinfo_SWAP_LOCK) {
			WeaverQinfo qinfo_0 = qinfo;
			// a swap is being performed by another thread
			while (true) {
				if (
					atomic_compare_exchange_strong(
						&rt->qinfo, &qinfo, qinfo - WeaverQinfo_RC_ONE
					)
				)
					break;

				if ((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)
					goto load_state;
			}

			// wait for swap to complete
			while (!((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK)) {
				CPU_PAUSE;
				qinfo = atomic_load(&rt->qinfo);
			}

			goto load_state;
		}

		// we have a valid reference and can read a task from the queue
		qinfo = atomic_fetch_add(&rt->qinfo, WeaverQinfo_POS_ONE);

		const usize pos = WeaverQinfo_pos(qinfo);
		const WeaverQueue *queue = rt->queue[qinfo & WeaverQinfo_INDEX_BIT];

		// check if queue is empty
		if (pos < queue->size) {
			pending_task = queue->tasks[pos];
			atomic_fetch_sub(&rt->qinfo, WeaverQinfo_RC_ONE);
			goto load_state;
		}

		// queue is empty, try to swap
		// release our reference
		qinfo = atomic_fetch_sub(&rt->qinfo, WeaverQinfo_RC_ONE);

		// try to swap if mqueue is not empty
		if (atomic_load(&rt->mqinfo) & WeaverQinfo_POS_MASK) {
			if (!Weaver_swap(rt, qinfo)) {
				WeaverQinfo qinfo_0 = qinfo;
				// wait for other thread to complete swap
				do {
					CPU_PAUSE;
					qinfo = atomic_load(&rt->qinfo);
				} while (!((qinfo ^ qinfo_0) & WeaverQinfo_INDEX_MASK));
			}

			goto load_state;
		}

		// else if mqueue is empty try going idle

		if (
			atomic_compare_exchange_strong(
				&this->state, &state, WeaverThreadState_IDLE
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

		if (atomic_fetch_sub(&rt->threads_sync, 1) == 1) {
			uint lock = Weaver_LOCK_JOIN;
			if (atomic_compare_exchange_strong(&rt->lock, &lock, Weaver_LOCK_NONE)) {
				umtx_wake(&rt->lock, UMTX_WAKE_ALL);
			}
		}

		while (atomic_load(&this->state) == WeaverThreadState_IDLE) {
			umtx_wait(&this->state, WeaverThreadState_IDLE);
		}

		atomic_fetch_add(&rt->threads_sync, 1);

		goto load_state;
	}

	if (0) Bdown: {
		this->orphaned_task = pending_task;

		if (atomic_load(&rt->threads_sync) == rt->threads_size) {
			uint lock = Weaver_LOCK_DOWN;
			if (atomic_compare_exchange_strong(&rt->lock, &lock, Weaver_LOCK_NONE)) {
				umtx_wake(&rt->lock, UMTX_WAKE_ALL);
			}
		}

		return nullptr;
	}

	UNREACHABLE;
}
