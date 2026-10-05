AsyncIntent AsyncFuture_resolve(AsyncFuture this, AsyncTask *out_task) {
	auto data = AsyncFuture_data(this);
	switch (AsyncFuture_class(this)) {
		case AsyncFutureClass_Mutex:
			return AsyncFuture_Mutex_resolve(data);
		case AsyncFutureClass_MutexCount:
			return AsyncFuture_MutexCount_resolve(data);
		case AsyncFutureClass_Task:
			return AsyncFuture_Task_resolve(data, out_task);
		case AsyncFutureClass_TaskCount:
			return AsyncFuture_TaskCount_resolve(data, out_task);
		default:;
			UNREACHABLE;
	}
}
