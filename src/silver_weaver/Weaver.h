#ifndef WEAVER_USE_PTHREAD
	#define WEAVER_USE_PTHREAD LIBSILVER_USE_PTHREAD
#endif

enum {
	WeaverState_None,
	WeaverState_Join,
	WeaverState_Down
};

typedef struct {
	_Alignas(cpu_hdi_size) struct {
		AsyncRT iface;
		Alc alc;
		u32 threads_size;
	};

	_Alignas(cpu_hdi_size) _Atomic u32_lf state;
	_Alignas(cpu_hdi_size) _Atomic u32_lf threads_sync;

	_Alignas(cpu_hdi_size) WeaverQinfo_Atomic qinfo;
	_Alignas(cpu_hdi_size) WeaverQinfo_Atomic mqinfo;

	WeaverQueue queues[2];

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
