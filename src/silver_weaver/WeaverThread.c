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

	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "[",id,"] boot\n");
	#endif

	#if Weaver_DEBUG && Weaver_USE_TLS
		t_WeaverThread_id = id;
	#endif

	load_state:;
	state = atom_get(&this->state, atom_acq);

	dispatch_state:;

	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "[",id,"] dispatch state ",state,"\n");
	#endif

	switch (state) {
		case WeaverThreadState_Down: goto Bdown;
		case WeaverThreadState_Idle: goto Bidle;
		case WeaverThreadState_Run: goto Brun;
		case WeaverThreadState_Error: goto Berror;
	}

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

			case AsyncIntent_Yield:
				pending_task = AsyncTask_null;
				goto load_state;

			case AsyncIntent_Error:
				TODO("handle error");

			default:
				UNREACHABLE;
		}

		pending_task = AsyncTask_null;
		auto res = Weaver_submit(rt, &result.task, 1);
		if (res) TODO("handle error");
		goto load_state;
	}

	if (0) fetch_task: {
		// acquire read reference
		WeaverQueueInfo rinfo = atom_add(&rt->read.info,
			WeaverQueue_Rc_one | WeaverQueue_Pos_one,
			atom_sync
		);

		if (rinfo & WeaverQueue_LockSwap) {
			wait_for_swap:;
			rinfo = atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

			// wait for swap to finish
			while (rinfo & WeaverQueue_LockSwap) {
				CPU_YIELD;
				rinfo = atom_get(&rt->read.info, atom_acq);
			}

			#if Weaver_FAST_FETCH
				goto fetch_task;
			#else
				goto load_state;
			#endif
		}

		// we have a valid reference and can read a task from the queue
		WeaverQueueSize const pos = FIELD_GET(WeaverQueue_Pos, rinfo);
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
		rinfo = atom_or(&rt->read.info, WeaverQueue_LockSwap, atom_sync);

		// another thread is already swapping
		if (rinfo & WeaverQueue_LockSwap)
			goto wait_for_swap;

		// set swap lock on write queue
		WeaverQueueInfo winfo = atom_or(&rt->write.info, WeaverQueue_LockSwap, atom_sync);
		WeaverQueueSize const wsize = FIELD_GET(WeaverQueue_Pos, winfo);

		// check if write queue is empty
		if (!wsize) {
			// release swap lock
			atom_and(&rt->write.info, ~WeaverQueue_LockSwap, atom_sync);
			// release reference and swap lock
			atom_sub(&rt->read.info, WeaverQueue_Rc_one | WeaverQueue_LockSwap, atom_sync);

			goto idle;
		}

		// wait for all references to write queue to be released
		while (winfo & WeaverQueue_Rc_mask) {
			CPU_YIELD;
			winfo = atom_get(&rt->write.info, atom_acq);
		}

		// wait for all references to read queue to be released
		// except the one we hold
		while (rinfo & WeaverQueue_Rc_gtone) {
			CPU_YIELD;
			rinfo = atom_get(&rt->read.info, atom_acq);
		}

		// perform swap
		Ptr tmp_queue = rt->write.queue;
		u32 tmp_capacity = rt->write.capacity;

		rt->write.queue = rt->read.queue;
		rt->write.capacity = rt->read.capacity;

		rt->read.size = wsize;
		rt->read.queue = tmp_queue;
		rt->read.capacity = tmp_capacity;

		atom_and(&rt->write.info, WeaverQueue_swapmask, atom_sync);
		atom_and(&rt->read.info, WeaverQueue_swapmask, atom_sync);

		Weaver_wake(rt, wsize);

		goto load_state;
	}

	if (0) Bidle: {
		// resubmit pending task if thread was requested to go idle
		if (!AsyncTask_isnull(pending_task)) {
			#if Weaver_DEBUG
				PRINTBP(128, Stdout, "[",id,"] resubmit\n");
			#endif

			auto res = Weaver_submit(rt, &pending_task, 1);
			if (res) TODO("handle error");
			pending_task = AsyncTask_null;
		}

		idle:;
		#if Weaver_DEBUG
			PRINTBP(128, Stdout, "[",id,"] going idle\n");
		#endif

		#if 0
			// set bit in runtime idle bitmap
			u8 const map_idx = id / 64;
			u64 const map_bit = 1ull << (id % 64);
			atom_or(&rt->idle_map[map_idx], map_bit, atom_sync);
		#endif

		// check if this is was the last running thread
		if (
			(
				atom_sub(&rt->pool_info, WeaverPoolInfo_Sync_one, atom_sync)
					& WeaverPoolInfo_Sync_mask
			)
				== WeaverPoolInfo_Sync_one
		) {
			#if Weaver_DEBUG
				PRINTBP(128, Stdout, "[",id,"] sync threads\n");
			#endif
			// if the runtime is waiting for all threads to join, wake it up
			u32 want = WeaverState_Join;
			if (atom_cmpx(&rt->state, &want, WeaverState_Sync, atom_sync, atom_acq)) {
				auto res = Async_wake32(&rt->state, Async_wakeall);
				if (res) TODO("handle error");
			}
		}

		#if Weaver_DEBUG
			PRINTBP(128, Stdout, "[",id,"] wait\n");
		#endif

		do switch (Async_wait32(&this->state, WeaverThreadState_Idle, 0)) {
			default: TODO("handle error");
			case AsyncWaitRes_Mismatch:
			case AsyncWaitRes_Ok:
		} while ((state = atom_get(&this->state, atom_acq)) == WeaverThreadState_Idle);

		#if 0
			// unset bit in idle bitmap
			atom_and(&rt->idle_map[map_idx], ~map_bit, atom_sync);
		#endif

		#if Weaver_DEBUG
			PRINTBP(128, Stdout, "[",id,"] wake\n");
		#endif

		atom_add(&rt->pool_info, WeaverPoolInfo_Sync_one, atom_sync);

		goto dispatch_state;
	}

	if (0) Bdown: {
		this->orphaned_task = pending_task;

		// if this is the last thread to go down
		if (atom_sub(&rt->down_sync, 1, atom_sync) == 1) {
			u32 want = WeaverState_Down;
			if (atom_cmpx(&rt->state, &want, WeaverState_Sync, atom_sync, atom_acq)) {
				auto res = Async_wake32(&rt->state, Async_wakeall);
				if (res) TODO("handle error");
			}
		}

		#if Weaver_USE_PTHREAD
			return nullptr;
		#else
			#error "unimplemented"
		#endif
	}

	if (0) Berror: TODO("handle error");

	UNREACHABLE;
}
