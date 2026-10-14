constexpr auto Weaver_maxthreads = WeaverThreadId_max;

/*
enum {
	FIELD_DEF(WeaverPoolInfo_Sync, WeaverThreadId_width),
	FIELD_DEF(WeaverPoolInfo_MapRc, WeaverThreadId_width),
	FIELD_DEF(WeaverPoolInfo_NeededThreads, WeaverThreadId_width + 1),
	FLAG_DEF(WeaverPoolInfo_MasterLock)
};

constexpr u32 WeaverPoolInfo_MapRc_one = FIELD_SET(WeaverPoolInfo_MapRc, 1);
constexpr u32 WeaverPoolInfo_MapRc_mask = FIELD_MASK(WeaverPoolInfo_MapRc);

constexpr u32 WeaverPoolInfo_Sync_one = FIELD_SET(WeaverPoolInfo_MapRc, 1);
constexpr u32 WeaverPoolInfo_Sync_mask = FIELD_MASK(WeaverPoolInfo_MapRc);

constexpr u32 WeaverPoolInfo_NeededThreads_one =
	FIELD_SET(WeaverPoolInfo_NeededThreads, 1);

constexpr u32 WeaverPoolInfo_NeededThreads_zero =
	FIELD_WIDTH(WeaverPoolInfo_NeededThreads);

*/

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
	Alc alc;
	u32 threads_size;

	// shared state
	Weaver_CACHE_ALIGN struct {
		// bitmap of idle threads
		//static_assert(Weaver_maxthreads <= (256));
		//_Atomic u64_lf idle_map[4];
		//_Atomic u32_lf pool_info;
		_Atomic u32_lf state;
		_Atomic u32_lf idle_sync;
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
		WeaverQueueSize size;
		WeaverQueueSize capacity;
	} write;

	WeaverThread threads[];
} Weaver;

usize Weaver_ZZallocsize(u32 threads_size) {
	return __builtin_offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size);
}

#define Weaver_ALLOCSIZE(threads_size) \
	(__builtin_offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size))


AsyncRT *Weaver_upcast(Weaver *this) {
	return &this->iface;
}

void Weaver_boot(Weaver *this) {
	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "Weaver_boot\n");
	#endif

	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_JoinIdle, atom_sync);
			if (state != WeaverState_Down)
				PANIC("state is not down");
		}
	#else
		this->state = WeaverState_Join;
	#endif

	auto const size = this->threads_size;

	// set idle sync
	#if Weaver_SAFE
		{
			auto idle_sync = atom_exchg(&this->idle_sync, size, atom_sync);
			if (idle_sync != 0)
				PANIC("idle_sync not zero")
		}
	#else
		this->idle_sync = size;
	#endif

	for (u32 i = 0; i < size; i++) {
		WeaverThread *thread = this->threads + i;
		#if Weaver_SAFE
			{
				auto state = atom_exchg(&thread->state, WeaverThreadState_Idle, atom_sync);
				if (state != WeaverThreadState_Down)
					PANIC("thread is not down");
			}
		#else
			thread->state = WeaverThreadState_Idle;
		#endif

		#if Weaver_DEBUG
			PRINTBP(128, Stdout, "Weaver_boot: create [",i,"]\n");
		#endif

		#if Weaver_USE_PTHREAD
			pthread_create(
				&thread->thread, nullptr,
				&WeaverThread_main, thread
			);
		#else
			#error "unimplemented"

		#endif
	}

	do switch (Async_wait32(&this->state, WeaverState_JoinIdle, 0)) {
		default: TODO("handle error");
		case AsyncWaitRes_Mismatch:
		case AsyncWaitRes_Ok:
	} while (atom_get(&this->state, atom_acq) == WeaverState_JoinIdle);

	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_Run, atom_sync);
			if (state != WeaverState_Sync)
				PANIC("invalid state after boot");

			auto sync = atom_get(&this->idle_sync, atom_acq);
			if (sync != 0)
				PANIC("idle_sync mismatch");
		}
	#else
		atom_set(&this->state, WeaverState_Run, atom_sync);
	#endif

	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "Weaver_boot: sync\n")
	#endif
}

void Weaver_join(Weaver *this) {
	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "Weaver_join\n");
	#endif

	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_JoinIdle, atom_sync);
			if (state != WeaverState_Run)
				PANIC("invalid state");
		}
	#else
		atom_set(&this->state, WeaverState_JoinIdle, atom_sync);
	#endif

	// check if all threads are already idle
	if (atom_get(&this->idle_sync, atom_acq) == 0) {
		#if Weaver_DEBUG
			PRINTBP(128, Stdout, "Weaver_join: all threads already idle\n");
		#endif

		#if Weaver_SAFE
			{
				auto state = atom_exchg(&this->state, WeaverState_JoinDown, atom_sync);
				if (state != WeaverState_JoinIdle)
					PANIC("invalid state")
			}
		#else
			atom_set(&this->state, WeaverState_JoinDown);
		#endif

		goto down;
	}

	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "Weaver_join: wait for idle\n");
	#endif

	do switch (Async_wait32(&this->state, WeaverState_JoinIdle, 0)) {
		default: TODO("handle error");
		case AsyncWaitRes_Mismatch:
		case AsyncWaitRes_Ok:
	} while (atom_get(&this->state, atom_acq) == WeaverState_JoinIdle);

	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_JoinDown, atom_sync);
			if (state != WeaverState_Sync)
				PANIC("invalid state");

			auto sync = atom_get(&this->idle_sync, atom_acq);
			if (sync != 0)
				PANIC("idle_sync mismatch");
		}
	#else
		atom_set(&this->state, WeaverState_JoinDown, atom_sync);
	#endif

	#if Weaver_DEBUG
		PRINTBP(128, Stdout, "Weaver_join: wake\n");
	#endif

	down:;

	#if Weaver_SAFE
		{
			auto sync = atom_exchg(&this->down_sync, this->threads_size, atom_sync);
			if (sync != 0)
				PANIC("down_sync non zero");
		}
	#else
		atom_set(&this->down_sync, this->threads_size, atom_sync);
	#endif


	auto it = this->threads;
	auto const end = it + this->threads_size;
	for (; it < end; it++) {
		#if Weaver_SAFE
			{
				auto state = atom_exchg(&it->state, WeaverThreadState_Down, atom_sync);
				if (state != WeaverThreadState_Idle)
					PANIC("invalid state");
			}
		#else
			atom_set(&it->state, WeaverThreadState_Down, atom_sync);
		#endif

		auto res = Async_wake32(&it->state, Async_wakeall);
		if (res) TODO("handle error");
	}

	do switch (Async_wait32(&this->state, WeaverState_JoinDown, 0)) {
		default: TODO("handle error");
		case AsyncWaitRes_Mismatch:
		case AsyncWaitRes_Ok:
	} while(atom_get(&this->state, atom_acq) == WeaverState_JoinDown);

	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_Down, atom_sync);
			if (state != WeaverState_Sync)
				PANIC("invalid state");

			auto sync = atom_get(&this->down_sync, atom_acq);
			if (sync != 0)
				PANIC("down_sync mismatch");
		}
	#else
		atom_set(&this->state, WeaverState_Down, atom_sync);
	#endif

	#if Weaver_USE_PTHREAD
		it = this->threads;
		for (; it < end; it++) {
			[[maybe_unused]]
			auto res = pthread_join(it->thread, nullptr);
			if (res) TODO("handle pthread_join error");
		}
	#endif
}

void Weaver_wake(Weaver *this, usize n) {
	// TODO implement waking threads selectively
	// based on available tasks and load heuristics

	auto it = this->threads;
	auto const end = it + this->threads_size;

	for (; n && (it < end); it++) {
		if (
			atom_exchg(&it->state, WeaverThreadState_Run, atom_sync)
				== WeaverThreadState_Idle
		) {
			Async_wake32(&it->state, Async_wakeall);
			n--;
		}
	}
}

AlcRes Weaver_submit(Ptr vthis, const AsyncTask *tasks, usize tasks_size_) {
	Weaver *const this = vthis;

	#if Weaver_SAFE
		if (tasks_size_ > WeaverQueueSize_max)
			return AlcRes_ErrAux_0;
	#endif

	auto const tasks_size = (WeaverQueueSize)tasks_size_;

	resubmit:;
	// acquire a write queue reference and insert position
	WeaverQueueInfo winfo = atom_add(&this->write.info,
		WeaverQueue_Rc_one | (WeaverQueue_Pos_one * tasks_size),
		atom_sync
	);

	// if swap or resize is being performed
	if (winfo & WeaverQueue_lockmask) {
		wait_for_locks:;
		// release our reference
		winfo = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);

		// wait for queue to unlock
		while (winfo & WeaverQueue_lockmask) {
			CPU_YIELD;
			winfo = atom_get(&this->write.info, atom_acq);
		}

		goto resubmit;
	}

	// we have a valid reference
	WeaverQueueSize const wpos = FIELD_GET(WeaverQueue_Pos, winfo);
	auto wcapacity = this->write.capacity;

	// if the position was already past the capacity
	// we know that a thread before us overflowed the queue first
	// and will handle the resize operation which will invalidate
	// our insert position
	if (wpos > wcapacity) goto wait_for_locks;

	// perform resize
	#if Weaver_SAFE
		usize const target_pos = (usize)wpos + (usize)tasks_size;
		if (target_pos > WeaverQueueSize_max)
			return AlcRes_ErrAux_1;

	#else
		auto const target_pos = wpos + tasks_size;
	#endif

	if (target_pos > wcapacity) {
		// lock write queue for resizing
		winfo = atom_or(&this->write.info, WeaverQueue_LockResize, atom_sync);

		// wait until all other references are released
		while (winfo & WeaverQueue_Rc_gtone) {
			CPU_YIELD;
			winfo = atom_get(&this->write.info, atom_acq);
		}

		// resize queue up to the prelock position
		usize const prelock_pos = FIELD_GET(WeaverQueue_Pos, winfo);

		usize capacity = wcapacity;
		// TODO add overflow checking
		do capacity *= 2;
		while (prelock_pos > capacity);

		#if Weaver_SAFE
			if (capacity > WeaverQueueSize_max)
				capacity = WeaverQueueSize_max;
		#endif

		usize alloc_size = usize_align(capacity * sizeof(AsyncTask), WeaverQueue_align);

		AlcReq req = {
			.intent = AlcIntent_Resize,
			.size = alloc_size,
			.align = WeaverQueue_align
		};

		AlcPtr ptr = Alc_invoke(this->alc, &req, nullptr, this->write.queue);
		auto res = AlcPtr_get(ptr);
		if (res) return res;

		capacity = req.size / sizeof(AsyncTask);
		if (capacity > WeaverQueueSize_max)
			capacity = WeaverQueueSize_max;

		this->write.queue = (Ptr)ptr;
		this->write.capacity = (WeaverQueueSize)capacity;

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
				) | target_pos,
				atom_sync, atom_acq
			)
		) {};
	}

	// insert tasks into queue
	memcpy(this->write.queue + wpos, tasks, sizeof(AsyncTask) * tasks_size);

	// release our reference
	winfo = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);

	// if another thread is performing a swap already
	// or there are other threads with valid references
	// we dont need to swap, the last thread will do the swap
	if (winfo & (WeaverQueue_LockSwap | WeaverQueue_Rc_gtone))
		return AlcRes_Ok;

	// get a reference to read queue
	WeaverQueueInfo rinfo = atom_add(&this->read.info, WeaverQueue_Rc_one, atom_sync);

	// if swap is being performed by other thread
	if (rinfo & WeaverQueue_LockSwap) {
		// release reference and return
		atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);
		return AlcRes_Ok;
	}

	// we have a valid read queue reference
	u32 rpos = FIELD_GET(WeaverQueue_Pos, rinfo);
	u32 const rsize = this->read.size;

	if (rpos < rsize) {
		// if read queue is not empty release reference and return
		atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);
		return AlcRes_Ok;
	}

	// set swap lock
	rinfo = atom_or(&this->read.info, WeaverQueue_LockSwap, atom_sync);
	if (rinfo & WeaverQueue_LockSwap) {
		atom_sub(&this->read.info, WeaverQueue_Rc_one, atom_sync);
		return AlcRes_Ok;
	}

	// set swap lock on write queue
	winfo = atom_or(&this->write.info, WeaverQueue_LockSwap, atom_sync);
	// remember the queue size before we locked it
	u32 const wsize = FIELD_GET(WeaverQueue_Pos, winfo);

	// if write queue is empty release locks and return
	if (!wsize) {
		atom_and(&this->write.info, ~WeaverQueue_LockSwap, atom_sync);
		// release reference and lock at the same time
		atom_sub(&this->read.info, WeaverQueue_Rc_one | WeaverQueue_LockSwap, atom_sync);
		return AlcRes_Ok;
	}

	// wait for all write queue references to be released
	while (winfo & WeaverQueue_Rc_mask) {
		CPU_YIELD;
		winfo = atom_get(&this->write.info, atom_acq);
	}

	// wait for all read queue references to be released
	// except the one we hold
	while (winfo & WeaverQueue_Rc_gtone) {
		CPU_YIELD;
		winfo = atom_get(&this->read.info, atom_acq);
	}

	// perform swap
	auto tmp_queue = this->write.queue;
	auto tmp_capacity = this->write.capacity;

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
	Weaver_wake(this, wsize);

	return AlcRes_Ok;
}

AlcPtr Weaver_alc(Ptr vthis, AlcReq *req, Ptr arg, Ptr mem) {
	Weaver *const this = vthis;
	return Alc_invoke(this->alc, req, arg, mem);
}

AlcRes Weaver_init(
	Weaver *this, Alc alc, u32 threads_size, WeaverQueueSize queue_capacity
) {
	if (threads_size > Weaver_maxthreads)
		return AlcRes_ErrAux_0;

	this->iface.alc = &Weaver_alc;
	this->iface.submit = &Weaver_submit;

	this->alc = alc;
	this->threads_size = threads_size;

	//memset(this->idle_map, 0, sizeof(this->idle_map));
	this->state = WeaverState_Down;
	this->idle_sync = 0; // setup in boot function
	this->down_sync = 0;


	usize queue_allocsize = usize_align(queue_capacity * sizeof(AsyncTask), WeaverQueue_align);

	AlcReq req = {
		.intent = AlcIntent_New,
		.size = queue_allocsize,
		.align = WeaverQueue_align
	};

	{ // read queue
		AlcPtr ptr = Alc_invoke(alc, &req, nullptr, nullptr);
		auto res = AlcPtr_get(ptr);
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
		AlcPtr ptr = Alc_invoke(alc, &req, nullptr, nullptr);
		auto res = AlcPtr_get(ptr);
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
		this->threads[i].id = (WeaverThreadId)i;
		this->threads[i].rt = this;
		this->threads[i].orphaned_task = AsyncTask_null;
		this->threads[i].state = WeaverThreadState_Down;
	}

	return AlcRes_Ok;
}

AlcRes Weaver_deinit(Weaver *this) {
	auto const alc = this->alc;

	auto res = Alc_delete(alc, this->read.queue);
	if (res) return res;

	res = Alc_delete(alc, this->write.queue);
	if (res) return res;

	return AlcRes_Ok;
}

AlcPtr Weaver_create(Alc alc, u32 threads_size, WeaverQueueSize queue_capacity) {
	AlcReq req = {
		.intent = AlcIntent_New,
		.size = Weaver_ZZallocsize(threads_size),
		.align = _Alignof(Weaver)
	};

	auto ptr = Alc_invoke(alc, &req, nullptr, nullptr);
	if (AlcPtr_check(ptr)) return ptr;

	auto this = (Weaver*)ptr;

	auto res = Weaver_init(this, alc, threads_size, queue_capacity);
	if (res) return AlcPtr_set(res);

	return (AlcPtr)this;
}

AlcRes Weaver_destroy(Weaver *this) {
	auto res = Weaver_deinit(this);
	if (res) return res;

	auto const alc = this->alc;
	res = Alc_delete(alc, this);

	return AlcRes_Ok;
}
