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
		WeaverQueue *queue;
	} read;

	Weaver_CACHE_ALIGN struct {
		WeaverQueueInfo_Atomic info;
		WeaverQueue *queue;
	} write;

	WeaverThread threads[];
} Weaver;

usize Weaver_ZZallocsize(u32 threads_size) {
	return __builtin_offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size);
}

AsyncRT *Weaver_upcast(Weaver *this) {
	return &this->iface;
}

AlcRes Weaver_submit(Ptr this, const AsyncTask *tasks, usize tasks_size) {

}

AlcRes Weaver_init(
	Weaver *this, Alc alc, u32 threads_size, usize queue_capacity
) {

}

bool Weaver_swap(Weaver *this, WeaverQueueInfo expected_index) {

}
