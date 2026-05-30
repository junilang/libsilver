void ZZAsyncFuture_Wake_resolve(_Atomic u32 *sp) {
	u32 state = AsyncFutureState_WAITING;
	if (atomic_compare_exchange_strong(sp, &state, AsyncFutureState_RESOLVED)) {
		syscall(SYS_futex, sp, FUTEX_WAKE, INT_MAX);
	}
}

void ZZAsyncFuture_Wake_wait(_Atomic u32 *sp) {
	u32 state = AsyncFutureState_NONE;
	if (atomic_compare_exchange_strong(sp, &state, AsyncFutureState_WAITING)) {
		do {
			syscall(SYS_futex, sp, FUTEX_WAIT, AsyncFutureState_WAITING, nullptr);
		} while (atomic_load(sp) == AsyncFutureState_WAITING);
	}
}

typedef struct {
	_Atomic u32 state;
	AsyncFuture_METADATA
} AsyncFuture_Wake;


AsyncResult AsyncFuture_Wake_callback(void *vthis) {
	auto const this = AsyncFuture_TODATA(AsyncFuture_Wake, vthis);
	ZZAsyncFuture_Wake_resolve(&this->state);
	return AsyncResult_YIELD;
}

AsyncFuture AsyncFuture_Wake_init(AsyncFuture_Wake *this) {
	atomic_init(&this->state, AsyncFutureState_NONE);
	return AsyncFuture_setmeta(
		AsyncFuture_TOMETA(this), AsyncFutureType_WAKE
	);
}

AsyncFuture AsyncFuture_Wake_upcast(AsyncFuture_Wake *this) {
	return AsyncFuture_upcast(
		AsyncFuture_TOMETA(this), AsyncFutureType_WAKE
	);
}

void AsyncFuture_Wake_wait(AsyncFuture_Wake *this) {
	ZZAsyncFuture_Wake_wait(&this->state);
}

typedef struct {
	_Atomic u32 state;
	_Atomic u32 count;
	AsyncFuture_METADATA
} AsyncFuture_CountWake;

AsyncResult AsyncFuture_CountWake_callback(void *vthis) {
	auto const this = AsyncFuture_TODATA(AsyncFuture_CountWake, vthis);
	if (atomic_fetch_sub(&this->count, 1) == 1) {
		ZZAsyncFuture_Wake_resolve(&this->state);
	}

	return AsyncResult_YIELD;
}

AsyncFuture AsyncFuture_CountWake_init(AsyncFuture_CountWake *this, u32 count) {
	atomic_init(&this->state, AsyncFutureState_NONE);
	atomic_init(&this->count, count);

	return AsyncFuture_setmeta(
		AsyncFuture_TOMETA(this), AsyncFutureType_COUNT_WAKE
	);
}

AsyncFuture AsyncFuture_CountWake_upcast(AsyncFuture_CountWake *this) {
	return AsyncFuture_upcast(
		AsyncFuture_TOMETA(this), AsyncFutureType_COUNT_WAKE
	);
}

void AsyncFuture_CountWake_wait(AsyncFuture_CountWake *this) {
	ZZAsyncFuture_Wake_wait(&this->state);
}
