#if WEAVER_USE_PTHREAD
	Ptr WeaverThread_main(Ptr vthis)
#else
	#error "unimplemented"
#endif

{
	WeaverThread *const this = vthis;
	auto const id = this->id;
	Weaver *const rt = this->rt;
	AsyncTask pending_task = AsyncTask_null;
	u32 state;


	#if Weaver_DEBUG && Weaver_USE_PTHREAD
		t_WeaverThread_id = id;
	#endif


	load_state:;
	state = atom_get(&this->state, atom_acq);

	dispatch_state:;
	switch (state) {
		case WeaverThreadState_Down: goto Bdown;
		case WeaverThreadState_Idle: goto Bidle;
		case WeaverThreadState_Run: goto Brun;
	}

	if (0) Bdown:;
	if (0) Bidle:;

	if (0) Brun: {
		if (AsyncTask_isnull(pending_task))
			goto fetch_task;

		AsyncResult result;
		switch (AsyncTask_call(pending_task, (AsyncRT*)rt, id, &result)) {
			case AsyncIntent_Call:
				#if Weaver_CALL_IMMEDIATE
					pending_task = result.task;
					break;
				#else
					goto submit_task;
				#endif

			case AsyncIntent_Resume:
				#if Weaver_RESUME_IMMEDIATE
					pending_task = result.task;
					break;
				#else
					goto submit_task;
				#endif

			case AsyncIntent_Suspend:
				goto submit_task;

			case AsyncIntent_Error:
				PANIC("TODO handle error");

			default:
				UNREACHABLE;
		}

		if (0) submit_task: {
			pending_task = AsyncTask_null;
			auto res = Weaver_submit(rt, &result.task, 1);
			if (res) PANIC("TODO handle error");
		}
	}

	if (0) fetch_task: {
		// acquire read reference
		WeaverQueueInfo info = atom_add(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

		if (info & WeaverQueue_LockSwap) {
			// if queue is locked for swapping we release our reference
			info = atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

			// wait for swap to finish
			while (info & WeaverQueue_LockSwap) {
				CPU_YIELD;
				info = atom_get(&rt->read.info, atom_sync);
			}

			goto fetch_task;
		}

		// we have a valid reference and can read a task from the queue
		// increment queue position by one
		info = atom_add(&rt->read.info, WeaverQueue_Pos_one, atom_acq);
		const usize pos = FIELD_GET(WeaverQueue_Pos, info);
		const WeaverQueue *queue = rt->read.queue;

		// check if queue has available tasks
		if (pos < queue->size) {
			pending_task = queue->tasks[pos];
			// release our reference
			atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_relaxed);
			goto load_state;
		}

		// release our reference
		info = atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

		// try to swap if write queue is not empty
		if (atom_get(&rt->write.info, atom_acq) & WeaverQueue_Pos_mask) {
			if (Weaver_swap(rt, info))
				goto load_state;

			// wait for other thread to complete swap
			auto const info_0 = info;
			do {
				CPU_YIELD;
				info = atom_get(&rt->read.info, atom_acq);
			} while (!((info ^ info_0) & WeaverQueue_Ticker_mask));

			goto load_state;
		}

		// go idle if no

	}

}
