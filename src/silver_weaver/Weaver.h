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
		u32 size;
		u32 capacity;
	} read;

	Weaver_CACHE_ALIGN struct {
		WeaverQueueInfo_Atomic info;
		AsyncTask *queue;
		u32 size;
		u32 capacity;
	} write;

	WeaverThread threads[];
} Weaver;

usize Weaver_ZZallocsize(u32 threads_size) {
	return __builtin_offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size);
}

AsyncRT *Weaver_upcast(Weaver *this) {
	return &this->iface;
}

AlcRes Weaver_submit(Ptr vthis, const AsyncTask *tasks, usize tasks_size) {
	Weaver *const this = vthis;

	resubmit:;
	// acquire a reference and insert position
	WeaverQueueInfo info = atom_add(&this->write.info,
		WeaverQueue_Rc_one | (WeaverQueue_Pos_one * tasks_size),
		atom_sync
	);

	// if swap or resize is being performed
	if (info & WeaverQueue_lockmask) {
		wait_for_locks:;
		// release our reference
		info = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);

		// wait for queue to unlock
		while (info & WeaverQueue_lockmask) {
			CPU_YIELD;
			info = atom_get(&this->write.info, atom_sync);
		}

		goto resubmit;
	}

	// we have a valid reference
	u32 const pos = FIELD_GET(WeaverQueue_Pos, info);
	auto const capacity = this->write.capacity;

	// if the position was already past the capacity
	// we know that a thread before us overflowed the queue first
	// and will handle the resize operation which will invalidate
	// our insert position
	if (pos > capacity)
		goto wait_for_locks;

	// perform resize
	if (pos + tasks_size > capacity) {
		// TODO
	}

	// insert tasks into queue
	memcpy(this->write.queue + pos, tasks, sizeof(AsyncTask) * tasks_size);

	// release our reference
	info = atom_sub(&this->write.info, WeaverQueue_Rc_one, atom_sync);

	// if another thread is performing a swap already
	// or there are other threads with valid references
	// we dont need to swap, the last thread will do the swap
	if (info & (WeaverQueue_LockSwap | WeaverQueue_Rc_gtone))
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
	info = atom_or(&this->write.info, WeaverQueue_LockSwap, atom_sync);
	u32 const queue_size = FIELD_GET(WeaverQueue_Pos, info);

	// if write queue is empty release locks and return
	if (!queue_size) {
		atom_and(&this->write.info, ~WeaverQueue_LockSwap, atom_sync);
		atom_and(&this->read.info, ~WeaverQueue_LockSwap, atom_sync);
		return AlcRes_Ok;
	}

	// wait for all write queue references to be released
	while (info & WeaverQueue_Rc_mask) {
		CPU_YIELD;
		info = atom_get(&this->write.info, atom_sync);
	}

	// wait for all read queue references to be released
	// except the one we hold
	while (info & WeaverQueue_Rc_gtone) {
		CPU_YIELD;
		info = atom_get(&this->read.info, atom_sync);
	}

	// perform swap
	Ptr tmp_queue = this->write.queue;
	u32 tmp_capacity = this->write.capacity;

	this->write.queue = this->read.queue;
	this->write.capacity = this->read.capacity;

	this->read.size = queue_size;
	this->read.queue = tmp_queue;
	this->read.capacity = tmp_capacity;

	atom_and(&this->write.info, WeaverQueue_swapmask, atom_sync);
	atom_and(&this->read.info, WeaverQueue_swapmask, atom_sync);

	// todo wake threads

	return AlcRes_Ok;
}

bool Weaver_swap(Weaver *this) {

}

AlcRes Weaver_init(
	Weaver *this, Alc alc, u32 threads_size, usize queue_capacity
) {

}
