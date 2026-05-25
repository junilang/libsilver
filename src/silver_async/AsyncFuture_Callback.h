typedef AsyncResult (*AsyncFuture_CallbackFn)(Ptr data);

typedef struct {
	AsyncFuture_CallbackFn callback;
	AsyncFuture_METADATA
} AsyncFuture_Callback;

AsyncResult AsyncFuture_Callback_callback(Ptr vthis) {
	auto const this = AsyncFuture_TODATA(AsyncFuture_Callback, vthis);
	return this->callback(this);
}

AsyncFuture AsyncFuture_Callback_upcast(AsyncFuture_Callback *this) {
	return AsyncFuture_setmeta(
		AsyncFuture_TOMETA(this), AsyncFutureType_CALLBACK
	);
}

typedef struct {
	AsyncFuture_CallbackFn callback;
	_Atomic u32 count;
	AsyncFuture_METADATA
} AsyncFuture_CountCallback;

AsyncResult AsyncFuture_CountCallback_callback(void *vthis) {
	auto const this = AsyncFuture_TODATA(AsyncFuture_CountCallback, vthis);
	if (atomic_fetch_sub(&this->count, 1) == 1) {
		return this->callback(this);
	}

	return AsyncResult_YIELD;
}

AsyncFuture AsyncFuture_CountCallback_upcast(AsyncFuture_CountCallback *this) {
	return AsyncFuture_setmeta(
		AsyncFuture_TOMETA(this),
		AsyncFutureType_COUNT_CALLBACK
	);
}
