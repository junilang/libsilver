enum {
	WeaverThreadState_Down,
	WeaverThreadState_Idle,
	WeaverThreadState_Run,
	WeaverThreadState_Boot
};

typedef union {
	_Alignas(cpu_hdi_size) char align__;
	struct {
		#if WEAVER_USE_PTHREAD
			pthread_t thread;
		#endif
		AsyncTask orphaned_task;
		_Atomic u32_lf state;
	};
} WeaverThread;
