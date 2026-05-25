typedef enum : u8 {
	WeaverThreadState_DOWN,
	WeaverThreadState_IDLE,
	WeaverThreadState_RUN,
	WeaverThreadState_BOOT
} WeaverThreadState;

typedef struct {
	pthread_t thread;
	union {
		// also used as futex
		_Atomic u32 state;
		AsyncTask orphaned_task;
	};
} WeaverThread;
