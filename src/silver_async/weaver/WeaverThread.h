enum : umtx {
	WeaverThread_DOWN,
	WeaverThread_IDLE,
	WeaverThread_RUN,
	WeaverThread_BOOT
};

typedef struct {
	pthread_t thread;
	AsyncTask orphaned_task;
	_Atomic umtx state;
} WeaverThread;
