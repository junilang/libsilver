#define AsyncFuture_wait(future) _Generic((future), \
	AsyncFuture_Mtx* : AsyncFuture_Mtx_wait, \
	AsyncFuture_MtxCount* : AsyncFuture_MtxCount_wait \
)(future)

#define AsyncFuture_set(future, task) _Generic((future), \
	AsyncFuture_Task* : AsyncFuture_Task_set, \
	AsyncFuture_TaskCount* : AsyncFuture_TaskCount_set \
)(future, task)

#define AsyncFuture_init(future, ...) _Generic((future), \
	AsyncFuture_Mtx* : AsyncFuture_Mtx_init, \
	AsyncFuture_MtxCount* : AsyncFuture_MtxCount_init, \
	AsyncFuture_Task* : AsyncFuture_Task_init, \
	AsyncFuture_TaskCount* : AsyncFuture_TaskCount_init \
)(future __VA_OPT__(,)__VA_ARGS__)

AsyncFuture ZZAsyncFuture_upcast_helper(AsyncFuture this) {
	return this;
}

#define AsyncFuture_upcast(future) _Generic((future), \
	AsyncFuture_Mtx* : AsyncFuture_Mtx_upcast, \
	AsyncFuture_MtxCount* : AsyncFuture_MtxCount_upcast, \
	AsyncFuture_Task* : AsyncFuture_Task_upcast, \
	AsyncFuture_TaskCount* : AsyncFuture_TaskCount_upcast, \
	AsyncFuture : ZZAsyncFuture_upcast_helper \
)(future)

AsyncIntent AsyncFuture_resolve(AsyncFuture this, AsyncResult *result) {
	Ptr data = AsyncFuture_data(this);
	switch (AsyncFuture_class(this)) {
		case AsyncFutureClass_MTX:
			return AsyncFuture_Mtx_resolve(data, result);
		case AsyncFutureClass_MTX_COUNT:
			return AsyncFuture_MtxCount_resolve(data, result);
		case AsyncFutureClass_TASK:
			return AsyncFuture_Task_resolve(data, result);
		case AsyncFutureClass_TASK_COUNT:
			return AsyncFuture_TaskCount_resolve(data, result);
		default:;
	}

	UNREACHABLE;
}
