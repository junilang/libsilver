enum {
	WeaverThreadState_Down,
	WeaverThreadState_Idle,
	WeaverThreadState_Run,

	// Boot state is last because states >= Boot describe thread id
	WeaverThreadState_Boot,
};

typedef union {
	Weaver_CACHE_ALIGN char align__;
	struct {
		#if WEAVER_USE_PTHREAD
			pthread_t thread;

		#else
			#error "unimplemented"

		#endif
		AsyncTask orphaned_task;
		_Atomic u32_lf state;
	};
} WeaverThread;
