enum {
	RioWorkerState_BOOT,
};

typedef struct {
	struct io_uring ring;
	pthread_t tpub;
	pthread_t tsub;
	_Atomic u32 state;
} RioWorker;

void RioWorker_init(RioWorker *this) {
}

Ptr RioWorker_tpub_main(Ptr vthis) {
	return nullptr;
}

Ptr RioWorker_tsub_main(Ptr vthis) {
	return nullptr;
}
