typedef enum : u8 {
	WeaverThreadState_DOWN,
	WeaverThreadState_IDLE,
	WeaverThreadState_RUN,
	WeaverThreadState_BOOT
} WeaverThreadState;

typedef struct {
	pthread_t thread;
	union {
		_Atomic umtx state;
		AsyncTask orphaned_task;
	};
} WeaverThread;
