constexpr auto Weaver_maxthreads = WeaverThreadId_max;

enum {
	FIELD_DEF(WeaverPoolInfo_IdleCount, WeaverThreadId_width),
	FIELD_DEF(WeaverPoolInfo_MapRc, WeaverThreadId_width),
	FIELD_DEF(WeaverPoolInfo_NeededThreads, WeaverThreadId_width + 1),
	FLAG_DEF(WeaverPoolInfo_MasterLock)
};

constexpr u32 WeaverPoolInfo_MapRc_one = FIELD_SET(WeaverPoolInfo_MapRc, 1);
constexpr u32 WeaverPoolInfo_MapRc_mask = FIELD_MASK(WeaverPoolInfo_MapRc);

constexpr u32 WeaverPoolInfo_IdleCount_one = FIELD_SET(WeaverPoolInfo_MapRc, 1);
constexpr u32 WeaverPoolInfo_IdleCount_mask = FIELD_MASK(WeaverPoolInfo_MapRc);

constexpr u32 WeaverPoolInfo_NeededThreads_one =
	FIELD_SET(WeaverPoolInfo_NeededThreads, 1);

constexpr u32 WeaverPoolInfo_NeededThreads_zero =
	FIELD_WIDTH(WeaverPoolInfo_NeededThreads);

enum {
	WeaverState_Run,
	WeaverState_Join, // main thread waiting for workers to go idle
	WeaverState_Sync,
	WeaverState_Down
};

typedef struct {
	// const state
	AsyncRT iface;
	Alc alc;
	u32 threads_size;

	// shared state
	Weaver_CACHE_ALIGN struct {
		// bitmap of idle threads
		static_assert(Weaver_maxthreads <= (256));
		_Atomic u64_lf idle_map[4];
		_Atomic u32_lf pool_info;
		_Atomic u32_lf state;
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

AsyncRT *Weaver_upcast(Weaver *this) {
	return &this->iface;
}

void Weaver_boot(Weaver *this) {
	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_Join, atom_sync);
			if (state != WeaverState_Down)
				PANIC("state is not down");
		}
	#else
		atom_set(&this->state, WeaverState_Join, atom_sync);
	#endif

	u32 const size = this->threads_size;
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

		#if Weaver_USE_PTHREAD
			pthread_create(
				&thread->thread, nullptr,
				&WeaverThread_main, thread
			);
		#else
			#error "unimplemented"

		#endif
	}

	do switch (Async_wait32(&this->state, WeaverState_Join, 0)) {
		case AsyncWaitRes_Mismatch:
			goto mismatch_break;
		default:;
	} while (atom_get(&this->state, atom_sync) == WeaverState_Join);

	mismatch_break:;

	#if Weaver_SAFE
		{
			auto state = atom_exchg(&this->state, WeaverState_Run, atom_sync);
			if (state != WeaverState_Sync)
				PANIC("invalid state after boot");

			auto pool = atom_get(&this->pool_info, atom_sync);
			if (FIELD_GET(WeaverPoolInfo_IdleCount, pool) != size)
				PANIC("idle count mismatch");
		}
	#else
		atom_set(&this->state, WeaverState_Run, atom_sync);
	#endif
}

void Weaver_wake(Weaver *this, usize n) {
	auto it = this->threads;
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
			winfo = atom_get(&this->write.info, atom_sync);
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
		// resize queue up to the prelock position
		usize const prelock_pos = FIELD_GET(WeaverQueue_Pos, winfo);

		// wait until all other references are released
		while (winfo & WeaverQueue_Rc_gtone) {
			CPU_YIELD;
			winfo = atom_get(&this->write.info, atom_sync);
		}

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
			!atom_cmpx(&this->write.info, &want,
				(want & FIELD_CLEAR(WeaverQueue_Pos)) | prelock_pos,
				atom_sync, atom_sync
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
		winfo = atom_get(&this->write.info, atom_sync);
	}

	// wait for all read queue references to be released
	// except the one we hold
	while (winfo & WeaverQueue_Rc_gtone) {
		CPU_YIELD;
		winfo = atom_get(&this->read.info, atom_sync);
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

	// todo wake threads

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

	memset(this->idle_map, 0, sizeof(this->idle_map));
	this->state = WeaverState_Down;
	this->pool_info = 0; // TODO setup


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
		this->threads[i].id = (WeaverThreadId)(i + 1);
		this->threads[i].rt = this;
		this->threads[i].orphaned_task = AsyncTask_null;
		this->threads[i].state = WeaverThreadState_Down;
	}

	return AlcRes_Ok;

}

AlcPtr Weaver_create(Alc alc, u32 threads_size, WeaverQueueSize queue_capacity) {

}
