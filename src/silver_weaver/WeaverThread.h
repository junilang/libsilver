// thread ids start at 1
typedef u8 WeaverThreadId;
constexpr auto WeaverThreadId_max = u8_max;
constexpr auto WeaverThreadId_width = 8;

enum {
	WeaverThreadState_Down,
	// WeaverThreadState_Boot,
	WeaverThreadState_Idle,
	WeaverThreadState_Run,
	WeaverThreadState_Error,
};

typedef struct {
	Weaver_CACHE_ALIGN struct {
		#if WEAVER_USE_PTHREAD
			pthread_t thread;
		#else
			#error "unimplemented"
		#endif
		Ptr rt; // const
		AsyncTask orphaned_task;
		_Atomic u32_lf state;
		WeaverThreadId id; // const
	};
} WeaverThread;

#if Weaver_DEBUG && Weaver_USE_PTHREAD
	_Thread_local u32 t_WeaverThread_id;
#endif

#if Weaver_USE_PTHREAD
	Ptr WeaverThread_main(Ptr);
#else
	#error "unimplemented"
#endif
