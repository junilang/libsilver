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
		case WeaverThreadState_Error: goto Berror;
	}

	if (0) Berror: UNREACHABLE;

	if (0) Bdown: {
		this->orphaned_task = pending_task;

		UNREACHABLE;
	}
	if (0) Bidle: UNREACHABLE;


	if (0) Brun: {
		if (AsyncTask_isnull(pending_task))
			goto fetch_task;

		AsyncResult result;
		switch (AsyncTask_call(pending_task, (AsyncRT*)rt, id, &result)) {
			case AsyncIntent_Call:
				#if Weaver_CALL_IMMEDIATE
					pending_task = result.task;
					goto load_state;
				#else
					break;
				#endif

			case AsyncIntent_Resume:
				#if Weaver_RESUME_IMMEDIATE
					pending_task = result.task;
					goto load_state;
				#else
					break;
				#endif

			case AsyncIntent_Suspend:
				break;

			case AsyncIntent_Error:
				PANIC("TODO handle error");

			default:
				UNREACHABLE;
		}

		pending_task = AsyncTask_null;
		auto res = Weaver_submit(rt, &result.task, 1);
		if (res) PANIC("TODO handle error");
		goto load_state;
	}

	if (0) fetch_task: {
		// acquire read reference
		WeaverQueueInfo info = atom_add(&rt->read.info,
			WeaverQueue_Rc_one | WeaverQueue_Pos_one,
			atom_sync
		);

		if (info & WeaverQueue_LockSwap) {
			wait_for_swap:;
			info = atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

			// wait for swap to finish
			while (info & WeaverQueue_LockSwap) {
				CPU_YIELD;
				info = atom_get(&rt->read.info, atom_sync);
			}

			#if Weaver_FAST_FETCH
				goto fetch_task;
			#else
				goto load_state;
			#endif
		}

		// we have a valid reference and can read a task from the queue
		u32 const pos = FIELD_GET(WeaverQueue_Pos, info);
		auto const size = rt->read.size;

		// check if queue is not empty
		if (pos < size) {
			pending_task = rt->read.queue[pos];
			// release reference
			atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

			#if Weaver_FAST_FETCH
				goto Brun;
			#else
				goto load_state;
			#endif
		}

		// queue is empty, attempt swap
		info = atom_or(&rt->read.info, WeaverQueue_LockSwap, atom_sync);

		// other thread is already swapping
		if (info & WeaverQueue_LockSwap)
			goto wait_for_swap;

		// set swap lock on write queue
		WeaverQueueInfo winfo = atom_or(&rt->write.info, WeaverQueue_LockSwap, atom_sync);
		u32 const queue_size = FIELD_GET(WeaverQueue_Pos, winfo);

		// check if write queue is empty
		if (!queue_size) {
			// release swap lock
			atom_and(&rt->write.info, ~WeaverQueue_LockSwap, atom_sync);
			atom_and(&rt->read.info, ~WeaverQueue_LockSwap, atom_sync);

			PANIC("TODO go idle");
		}

		// wait for all references to write queue to be released
		while (winfo & WeaverQueue_Rc_mask) {
			CPU_YIELD;
			winfo = atom_get(&rt->write.info, atom_sync);
		}

		// wait for all references to read queue to be released
		// except the one we hold
		while (info & WeaverQueue_Rc_gtone) {
			CPU_YIELD;
			info = atom_get(&rt->read.info, atom_sync);
		}

		// perform swap
		Ptr tmp_queue = rt->write.queue;
		u32 tmp_capacity = rt->write.capacity;

		rt->write.queue = rt->read.queue;
		rt->write.capacity = rt->read.capacity;

		rt->read.size = queue_size;
		rt->read.queue = tmp_queue;
		rt->read.capacity = tmp_capacity;

		atom_and(&rt->write.info, WeaverQueue_swapmask, atom_sync);
		atom_and(&rt->read.info, WeaverQueue_swapmask, atom_sync);

		goto load_state;
	}

}
