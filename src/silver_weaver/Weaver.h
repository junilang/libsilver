#ifndef Weaver_SAFE
	#define Weaver_SAFE LIBSILVER_SAFE
#endif

#ifndef Weaver_USE_PTHREAD
	#define Weaver_USE_PTHREAD LIBSILVER_USE_PTHREAD
#endif

#ifndef Weaver_CACHE_ALIGN
	#define Weaver_CACHE_ALIGN _Alignas(cpu_hdi_size)
#endif

#ifndef Weaver_CALL_IMMEDIATE
	#define Weaver_CALL_IMMEDIATE true
#endif

#ifndef Weaver_RESUME_IMMEDIATE
	#define Weaver_RESUME_IMMEDIATE true
#endif

#ifndef Weaver_DEBUG
	#define Weaver_DEBUG LIBSILVER_DEBUG
#endif

enum {
	WeaverState_None,
	WeaverState_Join,
	WeaverState_Down
};

constexpr u32 Weaver_maxthreads = FIELD_MAX(WeaverQinfo_Rc);

typedef struct {
	Weaver_CACHE_ALIGN struct {
		AsyncRT iface;
		Alc alc;
		u32 threads_size;

		_Atomic u32_lf state;
		_Atomic u32_lf threads_sync;
	};

	Weaver_CACHE_ALIGN struct {
		WeaverQinfo_Atomic info;
		WeaverQueue *queue;
	} read;

	Weaver_CACHE_ALIGN struct {
		WeaverQinfo_Atomic info;
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

AlcRes Weaver_submit(Ptr this, const AsyncTask *tasks, usize tasks_size);

AlcRes Weaver_init(
	Weaver *this, Alc alc, u32 threads_size, usize queue_capacity
) {

}
