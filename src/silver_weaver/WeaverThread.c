#if WEAVER_USE_PTHREAD
	Ptr WeaverThread_main(Ptr vthis)
#else
	#error "unimplemented"
#endif

{
	#if Weaver_DEBUG
		#define XDEBUG(...) PRINTBP(128, Weaver_DEBUG_STREAM, "[",id,"] ", __func__, ": " __VA_OPT__(,) __VA_ARGS__, "\n")
	#else
		#define XDEBUG(...)
	#endif

	WeaverThread *const this = vthis;
	auto const id = this->id;
	Weaver *const rt = this->rt;
	AsyncTask task = AsyncTask_null;
	u32 state;

	#if Weaver_USE_TLS
		t_WeaverThread_id = id;
	#endif

	XDEBUG("boot");

	load_state:;
	state = atom_get(&this->state, atom_acq);

	dispatch_state:;

	XDEBUG("dispatch state ", state);

	switch (state) {
		case WeaverThreadState_Down: goto Bdown;
		case WeaverThreadState_Idle: goto Bidle;
		case WeaverThreadState_Run: goto Brun;
		case WeaverThreadState_Error: goto Berror;
	}

	if (0) Brun: {
		if (AsyncTask_isnull(task))
			goto fetch_task;

		auto const task_res = AsyncTask_call(task, (AsyncRT*)rt, id, &task);

		switch (task_res) {
			case AsyncIntent_Call:
				#if Weaver_CALL_IMMEDIATE
					goto load_state;
				#else
					goto suspend_task;
				#endif

			case AsyncIntent_Resume:
				#if Weaver_RESUME_IMMEDIATE
					goto load_state;
				#else
					goto suspend_task;
				#endif

			case AsyncIntent_Suspend:
				goto suspend_task;

			case AsyncIntent_Yield:
				task = AsyncTask_null;
				goto load_state;

			default:;
		}

		PANIC("unhandled task error");

		if (0) suspend_task: {
			auto submit_res = Weaver_submit(rt, &task, 1);
			task = AsyncTask_null;
			if (submit_res) PANIC("unhandled submit error");
			goto load_state;
		}
	}

	if (0) fetch_task: {
		// acquire reference to read queue and task position
		WeaverQueueInfo rinfo = atom_add(&rt->read.info,
			WeaverQueue_Rc_one | WeaverQueue_Pos_one,
			atom_sync
		);

		if (rinfo & WeaverQueue_LockSwap) {
			wait_for_swap:;
			rinfo = atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

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
			task = rt->read.queue[pos];
			// release reference
			atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

			#if Weaver_FAST_FETCH
				goto Brun;
			#else
				goto load_state;
			#endif
		}

		// queue is empty, try to swap
		rinfo = atom_or(&rt->read.info, WeaverQueue_LockSwap, atom_sync);

		// another thread is already swapping
		if (rinfo & WeaverQueue_LockSwap)
			goto wait_for_swap;

		// set swap lock on write queue
		WeaverQueueInfo winfo = atom_or(&rt->write.info, WeaverQueue_LockSwap, atom_sync);

		// wait for all references to be released
		while (winfo & WeaverQueue_Rc_mask) {
			CPU_YIELD;
			winfo = atom_get(&rt->write.info, atom_acq);
		}

		// get write queue size
		auto const wsize = atom_exchg(&rt->write.size, 0, atom_sync);

		// cancel if write queue is empty
		if (!wsize) {
			// release references and locks if write queue is empty
			atom_and(&rt->write.info, ~WeaverQueue_LockSwap, atom_sync);
			atom_sub(&rt->read.info, WeaverQueue_Rc_one | WeaverQueue_LockSwap, atom_sync);

			// set state to idle if state didnt change
			if (atom_cmpx(&this->state, &state, WeaverThreadState_Idle, atom_sync, atom_acq))
				goto idle;
			else
				goto dispatch_state;
		}


		// wait for all references to read queue to be released
		// except the one we hold
		while (rinfo & WeaverQueue_Rc_gtone) {
			CPU_YIELD;
			rinfo = atom_get(&rt->read.info, atom_acq);
		}

		// perform swap
		auto const tmp_queue = rt->write.queue;
		auto const tmp_capacity = rt->write.capacity;

		rt->write.queue = rt->read.queue;
		rt->write.capacity = rt->read.capacity;

		rt->read.size = wsize;
		rt->read.queue = tmp_queue;
		rt->read.capacity = tmp_capacity;

		atom_and(&rt->write.info, WeaverQueue_swapmask, atom_sync);
		atom_and(&rt->read.info, WeaverQueue_swapmask, atom_sync);
		atom_sub(&rt->read.info, WeaverQueue_Rc_one, atom_sync);

		auto const wake_res = Weaver_wake(rt, wsize);
		if (wake_res) PANIC("unhandled wake error");

		goto load_state;
	}

	if (0) Bidle: {
		#if Weaver_SAFE
			ASSERT(
				AsyncTask_isnull(task)
				, "cannot go idle with non null task"
			);
		#endif

		idle:;
		XDEBUG("going idle state=", atom_get(&this->state, atom_acq));

		// check if this was the last running thread
		if (atom_sub(&rt->idle_sync, 1, atom_sync) == 1) {
			XDEBUG("sync threads");

			// if the runtime is waiting for all threads to join, wake it up
			u32 want = WeaverState_JoinIdle;
			if (atom_cmpx(&rt->state, &want, WeaverState_Sync, atom_sync, atom_acq)) {
				auto res = Async_wake32(&rt->state, Async_wakeall);
				if (res) PANIC("unhandled wake error");
			}
		}

		XDEBUG("idle wait")

		do {
			auto const res = Async_wait32(&this->state, WeaverThreadState_Idle, 0);
			switch (res) {
				default: PANIC("unhandle wait error");
				case AsyncWaitRes_Mismatch:
				case AsyncWaitRes_Ok:
			};

			state = atom_get(&this->state, atom_acq);
		} while (state == WeaverThreadState_Idle);


		XDEBUG("wake")

		atom_add(&rt->idle_sync, 1, atom_sync);
		atom_sub(&rt->wake_sync, 1, atom_sync);

		goto dispatch_state;
	}

	if (0) Bdown: {
		XDEBUG("down");

		this->orphaned_task = task;

		// if this is the last thread to go down
		if (atom_sub(&rt->down_sync, 1, atom_sync) == 1) {
			XDEBUG("down last");

			u32 want = WeaverState_JoinDown;
			if (atom_cmpx(&rt->state, &want, WeaverState_Sync, atom_sync, atom_acq)) {
				XDEBUG("down sync");

				auto const res = Async_wake32(&rt->state, Async_wakeall);
				if (res) PANIC("unhandled wake error");
			}
		}

		#if Weaver_USE_PTHREAD
			return nullptr;
		#else
			#error "unimplemented"
		#endif
	}

	if (0) Berror: PANIC("unhandled error");

	UNREACHABLE;

	#undef XDEBUG
}
