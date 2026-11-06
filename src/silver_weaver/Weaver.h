constexpr auto Weaver_maxthreads = WeaverThreadId_max;

enum {
	WeaverState_Down,
	WeaverState_Run,
	WeaverState_Sync,
	WeaverState_JoinIdle, // main thread waiting for workers to go idle
	WeaverState_JoinDown,
	WeaverState_Error,
};

typedef struct {
	// const state
	AsyncRT iface;
	Alc queue_alc;
	Alc provider_alc;
	u32 threads_size;

	// shared state
	Weaver_CACHE_ALIGN struct {
		// bitmap of idle threads
		//static_assert(Weaver_maxthreads <= (256));
		//_Atomic u64_lf idle_map[4];
		//_Atomic u32_lf pool_info;
		_Atomic u32_lf state;
		_Atomic u32_lf idle_sync;
		_Atomic u32_lf wake_sync;
		_Atomic u32_lf down_sync;
	};

	Weaver_CACHE_ALIGN struct {
		WeaverQueueInfo_Atomic info;
		AsyncTask *queue;
		WeaverQueueSize size;
		WeaverQueueSize capacity;
	} read;

	Weaver_CACHE_ALIGN struct {
		WeaverQueueInfo_Atomic info;
		AsyncTask *queue;
		WeaverQueueSize_Atomic size;
		WeaverQueueSize capacity;
	} write;

	WeaverThread threads[];
} Weaver;

usize Weaver_allocsize(u32 threads_size) {
	return __builtin_offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size);
}

#define Weaver_ALLOCSIZE(threads_size) \
	(__builtin_offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size))


AsyncRT *Weaver_upcast(Weaver *this) {
	return &this->iface;
}

#if Weaver_USE_TLS
	#define XTID t_WeaverThread_id
#else
	#define XTID "?"
#endif

#if Weaver_DEBUG
	#define XDEBUG(...) PRINTBA(128, Weaver_DEBUG_STREAM, "[",XTID,"] ", __func__, ": " __VA_OPT__(,) __VA_ARGS__, "\n")
#else
	#define XDEBUG(...)
#endif

[[nodiscard]]
AsyncRes Weaver_boot(Weaver *this) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, WeaverBoot);

	XDEBUG();

	#if Weaver_SAFE
		ASSERT(
			atom_exchg(&this->state, WeaverState_JoinIdle, atom_sync)
				== WeaverState_Down
			, "invalid state before boot"
		);
	#else
		this->state = WeaverState_Join;
	#endif

	auto const threads_size = this->threads_size;

	#if Weaver_SAFE
		ASSERT(
			atom_exchg(&this->idle_sync, threads_size, atom_sync) == 0
			, "idle count not zeroed"
		);
	#else
		this->idle_sync = threads_size;
	#endif

	for (u32 i = 0; i < threads_size; i++) {
		WeaverThread *thread = this->threads + i;
		#if Weaver_SAFE
			ASSERT(
				atom_exchg(&thread->state, WeaverThreadState_Idle, atom_sync)
					== WeaverThreadState_Down
				, "thread state is not down"
			);
		#else
			thread->state = WeaverThreadState_Idle;
		#endif

		XDEBUG("create thread ",i + 1);

		#if Weaver_USE_PTHREAD
		{
			// pthread should never return a negative
			// or an abnormally large error code value
			// that wouldn't fit inside AsyncRes_Code
			auto const res = pthread_create(
				&thread->thread, nullptr,
				&WeaverThread_main, thread
			);

			if (res) return (
				res_origin |
				FIELD(AsyncRes_Class, Pthread) |
				FIELD_SET(AsyncRes_Code, res)
			);
		}
		#else
			#error "unimplemented"

		#endif
	}

	XDEBUG("wait");

	do {
		auto const res = Async_wait32(&this->state, WeaverState_JoinIdle, nullptr);
		switch (res) {
			default:
				return (
					res_origin |
					FIELD(AsyncRes_Class, AsyncWaitRes) |
					FIELD_SET(AsyncRes_Code, res)
				);
			case AsyncWaitRes_Mismatch:
			case AsyncWaitRes_Ok:
		}
	} while (atom_get(&this->state, atom_acq) == WeaverState_JoinIdle);

	#if Weaver_SAFE
		ASSERT(
			atom_exchg(&this->state, WeaverState_Run, atom_sync)
				== WeaverState_Sync
			, "invalid state after boot"
		);

		ASSERT(
			atom_get(&this->idle_sync, atom_acq) == 0
			, "invalid idle count after boot"
		);
	#else
		atom_set(&this->state, WeaverState_Run, atom_sync);
	#endif

	XDEBUG("done");

	return AsyncRes_Ok;
}



[[nodiscard]]
AsyncRes Weaver_join(Weaver *this) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, WeaverBoot);

	XDEBUG();

	#if Weaver_SAFE
		ASSERT(
			atom_exchg(&this->state, WeaverState_JoinIdle, atom_sync)
				== WeaverState_Run
			, "invalid state before join"
		)
	#else
		atom_set(&this->state, WeaverState_JoinIdle, atom_sync);
	#endif

	auto const threads_size = this->threads_size;

	// TODO check this->wake_sync !!!!
	// !!!!
	// !!!!

	// check if all threads are already idle
	auto it = this->threads;
	auto const end = it + threads_size;
	for (; it < end; it++) {
		if (
			atom_get(&it->state, atom_acq) !=
				WeaverThreadState_Idle
		)
			goto wait;
	}

	XDEBUG("all threads idle");

	#if Weaver_SAFE
	{
		auto const state = atom_exchg(&this->state, WeaverState_JoinDown, atom_sync);
		// edge case where all threads are detected as being idle but
		// a thread was still in the process of going idle and changed
		// our join state to sync could occur here
		ASSERT(
			(state == WeaverState_JoinIdle) || (state == WeaverState_Sync)
			, "invalid state"
		);
	}
	#else
		atom_set(&this->state, WeaverState_JoinDown, atom_sync);
	#endif

	goto down;

	wait:;
	XDEBUG("wait for idle");

	do {
		auto const res = Async_wait32(&this->state, WeaverState_JoinIdle, nullptr);
		switch (res) {
			default: return (
				res_origin |
				FIELD(AsyncRes_Class, AsyncWaitRes) |
				FIELD_SET(AsyncRes_Code, res)
			);
			case AsyncWaitRes_Mismatch:
			case AsyncWaitRes_Ok:
		}
	} while (atom_get(&this->state, atom_acq) == WeaverState_JoinIdle);

	#if Weaver_SAFE
		ASSERT(
			atom_exchg(&this->state, WeaverState_JoinDown, atom_sync)
				== WeaverState_Sync
			, "invalid state after wake"
		);

		ASSERT(
			atom_get(&this->idle_sync, atom_acq) == 0
			, "invalid idle count"
		);
	#else
		atom_set(&this->state, WeaverState_JoinDown, atom_sync);
	#endif

	XDEBUG("wake");

	down:;

	#if Weaver_SAFE
		ASSERT(
			atom_exchg(&this->down_sync, threads_size, atom_sync) == 0
			, "down sync is not zero"
		);
	#else
		atom_set(&this->down_sync, threads_size, atom_sync);
	#endif

	it = this->threads;
	for (; it < end; it++) {
		#if Weaver_SAFE
		{
			auto const state = atom_exchg(&it->state, WeaverThreadState_Down, atom_sync);
			ASSERT_PRINT(
				state == WeaverThreadState_Idle
				, "invalid state(",state,") on thread [",((it - this->threads) + 1),"]"
			);
		}
		#else
			atom_set(&it->state, WeaverThreadState_Down, atom_sync);
		#endif

		auto const res = Async_wake32(&it->state, Async_wakeall);
		if (res) return (
			res_origin |
			FIELD(AsyncRes_Class, AsyncWaitRes) |
			FIELD_SET(AsyncRes_Code, res)
		);
	}

	do {
		auto const res = Async_wait32(&this->state, WeaverState_JoinDown, nullptr);
		switch (res) {
			default: return (
				res_origin |
				FIELD(AsyncRes_Class, AsyncWaitRes) |
				FIELD_SET(AsyncRes_Code, res)
			);
			case AsyncWaitRes_Mismatch:
			case AsyncWaitRes_Ok:
		}
	} while(atom_get(&this->state, atom_acq) == WeaverState_JoinDown);

	#if Weaver_SAFE
	{
		ASSERT(
			atom_exchg(&this->state, WeaverState_Down, atom_sync)
				== WeaverState_Sync
			, "invalid state after wake"
		);

		ASSERT(
			atom_get(&this->down_sync, atom_acq) == 0
			, "down sync not zero"
		);
	}
	#else
		atom_set(&this->state, WeaverState_Down, atom_sync);
	#endif

	#if Weaver_USE_PTHREAD
		it = this->threads;
		for (; it < end; it++) {
			// previous note on pthread return codes applies (in Weaver_boot)
			auto const res = pthread_join(it->thread, nullptr);
			if (res) return (
				res_origin |
				FIELD(AsyncRes_Class, Pthread) |
				FIELD_SET(AsyncRes_Code, res)
			);
		}
	#endif

	return AsyncRes_Ok;
}



[[nodiscard]]
AsyncRes Weaver_wake(Weaver *this, WeaverThreadId caller_id, usize nwake) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, WeaverWake);
	XDEBUG("nwake=",nwake);

	auto const threads_size = this->threads_size;
	for (u32 i = 0; i < threads_size && nwake; i++) {
		auto const id = (WeaverThreadId)(i + 1);

		// dont wake up caller thread
		if (id == caller_id) continue;

		auto const it = &this->threads[i];
		nwake--;

		if (
			atom_exchg(&it->state, WeaverThreadState_Run, atom_sync)
				== WeaverThreadState_Idle
		) {
			atom_add(&this->wake_sync, 1, atom_sync);
			auto const res = Async_wake32(&it->state, Async_wakeall);
			if (res) return (
				res_origin |
				FIELD(AsyncRes_Class, AsyncWaitRes) |
				FIELD_SET(AsyncRes_Code, res)
			);
		}

	}

	return AsyncRes_Ok;
}



[[nodiscard]]
AsyncRes Weaver_submit(Ptr vthis, usize context, const AsyncTask *tasks, usize tasks_size_) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, WeaverSubmit);
	constexpr AsyncRes res_toomanytasks = (
		res_origin |
		FIELD(AsyncRes_Class, WeaverRes) |
		FIELD_SET(AsyncRes_Code, WeaverRes_ErrTooManyTasks)
	);

	constexpr AsyncRes res_queueoverflow = (
		res_origin |
		FIELD(AsyncRes_Class, WeaverRes) |
		FIELD_SET(AsyncRes_Code, WeaverRes_QueueOverflow)
	);

	XDEBUG("tasks_size=",tasks_size_);

	Weaver *const this = vthis;

	#if Weaver_SAFE
		if (tasks_size_ > WeaverQueueSize_max)
			return res_toomanytasks;
	#endif

	auto const tasks_size = (WeaverQueueSize)tasks_size_;

	retry:;
	// acquire a write queue reference and insert position
	WeaverQueueInfo winfo = atom_add(&this->write.info,
		WeaverQueue_Rc_one | (WeaverQueue_Pos_one * tasks_size),
		atom_sync
	);

	// if swap or resize is being performed
	if (winfo & WeaverQueue_lockmask) {
		// release our reference and subtract position
		// subtracting position here makes a position overflow
		// on the queue info unlikely
		winfo = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);

		// wait for queue to unlock
		while (winfo & WeaverQueue_lockmask) {
			CPU_YIELD;
			winfo = atom_get(&this->write.info, atom_acq);
		}

		goto retry;
	}

	// we have a valid reference
	WeaverQueueSize const wpos = FIELD_GET(WeaverQueue_Pos, winfo);
	auto const wcapacity = this->write.capacity;
	auto wqueue = this->write.queue;

	// if the position was already past the capacity
	// we know that a thread before us overflowed the queue first
	// and will handle the resize operation which will invalidate
	// our insert position
	if (wpos > wcapacity) {
		#if Weaver_SAFE
			// wait for the other thread to actually set
			// the resize lock while we have our reference
			do {
				CPU_YIELD;
				winfo = atom_get(&this->write.info, atom_acq);
			} while (!(winfo & WeaverQueue_LockResize));
		#endif

		// release our reference
		winfo = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);
		// wait for queue to unlock
		while (winfo & WeaverQueue_lockmask) {
			CPU_YIELD;
			winfo = atom_get(&this->write.info, atom_acq);
		}

		goto retry;
	}

	WeaverQueueSize target_pos;
	#if Weaver_SAFE
	{
		auto const target_pos_ = (usize)wpos + (usize)tasks_size;
		// TODO invalidate global weaver state
		// if this condition occured, the queue position has overflowed
		if (target_pos_ > WeaverQueueSize_max)
			return res_queueoverflow;

		target_pos = (WeaverQueueSize)target_pos_;
	}
	#else
		auto const target_pos = wpos + tasks_size;
	#endif

	// perform resize if target_pos is beyond capacity
	if (target_pos > wcapacity) {
		// lock write queue for resizing
		winfo = atom_or(&this->write.info, WeaverQueue_LockResize, atom_sync);

		// wait until all other references are released
		while (winfo & WeaverQueue_Rc_gtone) {
			CPU_YIELD;
			winfo = atom_get(&this->write.info, atom_acq);
		}

		usize new_capacity = wcapacity;

		// enlarge queue until it fits target_pos
		do {
			new_capacity *= 2;
			#if Weaver_SAFE
				if (new_capacity > WeaverQueueSize_max)
					return res_queueoverflow;
			#endif
		} while (target_pos > new_capacity);

		auto const alloc_size = usize_align(new_capacity * sizeof(AsyncTask), WeaverQueue_align);

		AlcReq req = {
			.intent = AlcIntent_Resize,
			.size = alloc_size,
			.align = WeaverQueue_align
		};

		AlcPtr ptr = Alc_invoke(this->queue_alc, &req, nullptr, wqueue);
		auto const res = AlcPtr_get(ptr);
		if (res) return (
			res_origin |
			FIELD(AsyncRes_Class, AlcRes) |
			FIELD_SET(AsyncRes_Code, res)
		);

		new_capacity = req.size / sizeof(AsyncTask);
		#if Weaver_SAFE
			if (new_capacity > WeaverQueueSize_max)
				new_capacity = WeaverQueueSize_max;
		#endif

		wqueue = (Ptr)ptr;
		this->write.queue = wqueue;
		this->write.capacity = (WeaverQueueSize)new_capacity;

		// restore position to our state
		while (
			!atom_cmpx(&this->write.info, &winfo,
				(winfo &
					// clear position and resize lock
					(
						FIELD_CLEAR(WeaverQueue_Pos) |
						FLAG_CLEAR(WeaverQueue_LockResize)
					)
					// reset position back to the position we acquired
				) | FIELD_SET(WeaverQueue_Pos, target_pos),
				atom_sync, atom_acq
			)
		) {};
	}

	// insert tasks into queue and increment queue size
	memcpy(this->write.queue + wpos, tasks, sizeof(AsyncTask) * tasks_size);
	atom_add(&this->write.size, tasks_size, atom_sync);

	// release our reference
	winfo = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);

	// if another thread is performing a swap already
	// or there are other threads with valid references
	// we dont need to swap, the last thread will do the swap
	//
	// TODO analyze whether leaving swapping to the last submitting
	// thread does not starve worker threads, or add a configuration
	// option for this behavior
	if (winfo & (WeaverQueue_LockSwap | WeaverQueue_Rc_gtone))
		return AsyncRes_Ok;

	// try performing a swap
	// get a reference to read queue
	// the read queue swap lock takes priority over the write queue
	WeaverQueueInfo rinfo = atom_add(&this->read.info, WeaverQueue_Rc_one, atom_sync);

	// if swap is being performed by other thread
	if (rinfo & WeaverQueue_LockSwap) {
		// release reference and return
		atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);
		return AsyncRes_Ok;
	}

	// we have a valid read queue reference and can check
	// whether the queue is empty by comparing position and size
	WeaverQueueSize const rpos = FIELD_GET(WeaverQueue_Pos, rinfo);
	auto const rsize = this->read.size;
	if (rpos < rsize) {
		// if read queue is not empty release reference and return
		atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);
		return AsyncRes_Ok;
	}

	// try to set read queue swap lock
	rinfo = atom_or(&this->read.info, WeaverQueue_LockSwap, atom_sync);
	if (rinfo & WeaverQueue_LockSwap) {
		// cancel if swap is being performed by other thread
		atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);
		return AsyncRes_Ok;
	}

	// set swap lock on write queue
	winfo = atom_or(&this->write.info, WeaverQueue_LockSwap, atom_sync);

	// wait for all write queue references to be released
	while (winfo & WeaverQueue_Rc_mask) {
		CPU_YIELD;
		winfo = atom_get(&this->write.info, atom_acq);
	}

	// exchange write queue size for 0
	// we would need to zero it anyway after swapping
	auto const wsize = atom_exchg(&this->write.size, 0, atom_sync);

	if (!wsize) {
		// release references and locks if write queue is empty
		atom_and(&this->write.info, ~WeaverQueue_LockSwap, atom_sync);
		atom_sub(&this->read.info, WeaverQueue_Rc_one | WeaverQueue_LockSwap, atom_sync);
		return AsyncRes_Ok;
	}

	// wait for all read queue references to be released
	// except the one we hold
	while (rinfo & WeaverQueue_Rc_gtone) {
		CPU_YIELD;
		winfo = atom_get(&this->read.info, atom_acq);
	}

	// perform swap
	auto const tmp_queue = this->write.queue;
	auto const tmp_capacity = this->write.capacity;

	this->write.queue = this->read.queue;
	this->write.capacity = this->read.capacity;

	this->read.size = wsize;
	this->read.queue = tmp_queue;
	this->read.capacity = tmp_capacity;

	// clear locks and release reference
	atom_and(&this->write.info, WeaverQueue_swapmask, atom_sync);
	atom_and(&this->read.info, WeaverQueue_swapmask, atom_sync);
	atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);

	// wake threads
	return Weaver_wake(this, (WeaverThreadId)context, wsize);
}

#if Alc_PTRTAG
	Alc Weaver_alc(Ptr vthis, usize context) {
		auto const this = (Weaver*)vthis;
		return this->provider_alc;
	}
#else
	void Weaver_alc(Ptr vthis, usize context, Alc *out_alc) {
		auto const this = (Weaver*)vthis;
		*out_alc = this->provider_alc;
	}
#endif

[[nodiscard]]
AlcRes Weaver_init(
	Weaver *this, Alc queue_alc, Alc provider_alc, u32 threads_size, WeaverQueueSize queue_capacity
) {
	#if Weaver_SAFE
		ASSERT(threads_size <= Weaver_maxthreads);
	#endif

	this->iface.submit = &Weaver_submit;
	this->iface.alc = &Weaver_alc;

	this->queue_alc = queue_alc;
	this->provider_alc = provider_alc;
	this->threads_size = threads_size;

	this->state = WeaverState_Down;
	this->idle_sync = 0; // setup in boot function
	this->down_sync = 0;
	this->wake_sync = 0;


	usize queue_allocsize = usize_align(queue_capacity * sizeof(AsyncTask), WeaverQueue_align);

	AlcReq req = {
		.intent = AlcIntent_New,
		.size = queue_allocsize,
		.align = WeaverQueue_align
	};

	{ // read queue
		AlcPtr ptr = Alc_invoke(queue_alc, &req, nullptr, nullptr);
		auto const res = AlcPtr_get(ptr);
		if (res) return res;

		usize capacity = req.size / sizeof(AsyncTask);
		if (capacity > WeaverQueueSize_max)
			capacity = WeaverQueueSize_max;

		this->read.queue = (Ptr)ptr;
		this->read.capacity = (WeaverQueueSize)(capacity);
		this->read.info = 0;
		this->read.size = 0;
	}

	{ // write queue
		req.size = queue_allocsize;
		AlcPtr ptr = Alc_invoke(queue_alc, &req, nullptr, nullptr);
		auto const res = AlcPtr_get(ptr);
		if (res) return res;

		usize capacity = req.size / sizeof(AsyncTask);
		if (capacity > WeaverQueueSize_max)
			capacity = WeaverQueueSize_max;

		this->write.queue = (Ptr)ptr;
		this->write.capacity = (WeaverQueueSize)(capacity);
		this->write.info = 0;
		this->write.size = 0;
	}

	// setup threads

	for (u32 i = 0; i < threads_size; i++) {
		this->threads[i].id = (WeaverThreadId)i + 1;
		this->threads[i].rt = this;
		this->threads[i].orphaned_task = AsyncTask_null;
		this->threads[i].state = WeaverThreadState_Down;
	}

	return AlcRes_Ok;
}

[[nodiscard]]
AlcRes Weaver_deinit(Weaver *this) {
	auto const alc = this->queue_alc;

	auto res = Alc_delete(alc, this->read.queue);
	if (res) return res;

	res = Alc_delete(alc, this->write.queue);
	if (res) return res;

	return AlcRes_Ok;
}

[[nodiscard]]
AlcPtr Weaver_create(Alc alc, Alc provider_alc, u32 threads_size, WeaverQueueSize queue_capacity) {
	AlcReq req = {
		.intent = AlcIntent_New,
		.size = Weaver_allocsize(threads_size),
		.align = _Alignof(Weaver)
	};

	auto ptr = Alc_invoke(alc, &req, nullptr, nullptr);
	if (AlcPtr_check(ptr)) return ptr;

	auto this = (Weaver*)ptr;

	auto res = Weaver_init(this, alc, provider_alc, threads_size, queue_capacity);
	if (res) return AlcPtr_set(res);

	return (AlcPtr)this;
}

[[nodiscard]]
AlcRes Weaver_destroy(Weaver *this) {
	auto res = Weaver_deinit(this);
	if (res) return res;

	auto const alc = this->queue_alc;
	res = Alc_delete(alc, this);

	return AlcRes_Ok;
}

#undef XDEBUG
#undef XTID
