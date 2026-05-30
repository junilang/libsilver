AsyncResult ZZAsyncFuture_Resume_resolve(_Atomic u64 *sp) {
	u64 state = AsyncFutureState_NONE;
	if (atomic_compare_exchange_strong(sp, &state, AsyncFutureState_RESOLVED)) {
		return AsyncResult_YIELD;
	}

	return (AsyncResult){.next.value = (Ptr)state};
}

bool ZZAsyncFuture_Resume_set(_Atomic u64 *sp, AsyncTask task) {
	u64 state = AsyncFutureState_NONE;
	return atomic_compare_exchange_strong(sp, &state, (u64)task.value);
}

typedef struct {
	_Atomic u64 state;
	AsyncFuture_METADATA
} AsyncFuture_Resume;

AsyncResult AsyncFuture_Resume_callback(Ptr vthis) {
	auto const this = AsyncFuture_TODATA(AsyncFuture_Resume, vthis);
	return ZZAsyncFuture_Resume_resolve(&this->state);
}

AsyncFuture AsyncFuture_Resume_init(AsyncFuture_Resume *this) {
	atomic_init(&this->state, AsyncFutureState_NONE);
	return AsyncFuture_setmeta(AsyncFuture_TOMETA(this), AsyncFutureType_RESUME);
}

bool AsyncFuture_Resume_set(AsyncFuture_Resume *this, AsyncTask task) {
	return ZZAsyncFuture_Resume_set(&this->state, task);
}

AsyncFuture AsyncFuture_Resume_upcast(AsyncFuture_Resume *this) {
	return AsyncFuture_upcast(
		AsyncFuture_TOMETA(this), AsyncFutureType_RESUME
	);
}

typedef struct {
	_Atomic u64 state;
	_Atomic u32 count;
	AsyncFuture_METADATA
} AsyncFuture_CountResume;

AsyncResult AsyncFuture_CountResume_callback(Ptr vthis) {
	auto const this = AsyncFuture_TODATA(AsyncFuture_CountResume, vthis);

	auto const cnt = atomic_fetch_sub(&this->count, 1);
	//FPRINT_ATOMIC(stderr, __func__,": ",(Ptr)&this->state," cnt=",cnt,"\n");

	if (cnt == 1) {
		return ZZAsyncFuture_Resume_resolve(&this->state);
	}

	return AsyncResult_YIELD;
}

AsyncFuture AsyncFuture_CountResume_init(AsyncFuture_CountResume *this, u32 count) {
	atomic_init(&this->state, AsyncFutureState_NONE);
	atomic_init(&this->count, count);
	return AsyncFuture_setmeta(
		AsyncFuture_TOMETA(this), AsyncFutureType_COUNT_RESUME
	);
}

bool AsyncFuture_CountResume_set(AsyncFuture_CountResume *this, AsyncTask task) {
	return ZZAsyncFuture_Resume_set(&this->state, task);
}

AsyncFuture AsyncFuture_CountResume_upcast(AsyncFuture_CountResume *this) {
	return AsyncFuture_upcast(
		AsyncFuture_TOMETA(this), AsyncFutureType_COUNT_RESUME
	);
}
