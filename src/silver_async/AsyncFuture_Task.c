enum : u32 {
	AsyncFuture_TaskState_NONE,
	AsyncFuture_TaskState_SET,
	AsyncFuture_TaskState_RESOLVED,
};

typedef union {
	alignas(AsyncFuture_ALIGN) char align__;
	struct {
		AsyncTask task;
		_Atomic u32 state;
	};
} AsyncFuture_Task;

AsyncFuture AsyncFuture_Task_upcast(AsyncFuture_Task *this) {
	return AsyncFuture_tag(this, AsyncFutureClass_TASK);
}

AsyncFuture AsyncFuture_Task_init(AsyncFuture_Task *this) {
	this->state = AsyncFuture_TaskState_NONE;
	return AsyncFuture_Task_upcast(this);
}

AsyncIntent AsyncFuture_Task_resolve(AsyncFuture_Task *this, AsyncTask *out_task) {
	if (
		atomic_exchange_explicit(
			&this->state, AsyncFuture_TaskState_RESOLVED, memory_order_acq_rel
		) == AsyncFuture_TaskState_SET
	) {
		*out_task = this->task;
		return AsyncIntent_RESUME;
	}

	return AsyncIntent_YIELD;
}

bool AsyncFuture_Task_set(AsyncFuture_Task *this, AsyncTask task) {
	this->task = task;

	umtx want = AsyncFuture_TaskState_NONE;
	if (
		atomic_compare_exchange_strong_explicit(
			&this->state, &want, AsyncFuture_TaskState_SET,
			memory_order_acq_rel, memory_order_acquire
		)
	)
		return true;

	#if BUILD_SAFE
	else if (want == AsyncFuture_TaskState_SET)
		PANIC("already set");
	#endif

	return false;
}

typedef union {
	alignas(AsyncFuture_ALIGN) char align__;
	struct {
		AsyncTask task;
		_Atomic u32 state;
		_Atomic u32 count;
	};
} AsyncFuture_TaskCount;

AsyncFuture AsyncFuture_TaskCount_upcast(AsyncFuture_TaskCount *this) {
	return AsyncFuture_tag(this, AsyncFutureClass_TASK_COUNT);
}

AsyncFuture AsyncFuture_TaskCount_init(AsyncFuture_TaskCount *this, u32 count) {
	this->state = AsyncFuture_TaskState_NONE;
	this->count = count;
	return AsyncFuture_TaskCount_upcast(this);
}

AsyncIntent AsyncFuture_TaskCount_resolve(AsyncFuture_TaskCount *this, AsyncTask *out_task) {
	if (atomic_fetch_sub_explicit(&this->count, 1, memory_order_acq_rel) != 1) {
		return AsyncIntent_YIELD;
	}

	if (
		atomic_exchange_explicit(
			&this->state, AsyncFuture_TaskState_RESOLVED, memory_order_release
		) == AsyncFuture_TaskState_SET
	) {
		*out_task = this->task;
		return AsyncIntent_RESUME;
	}

	return AsyncIntent_YIELD;
}

bool AsyncFuture_TaskCount_set(AsyncFuture_TaskCount *this, AsyncTask task) {
	this->task = task;

	umtx want = AsyncFuture_TaskState_NONE;
	if (
		atomic_compare_exchange_strong_explicit(
			&this->state, &want, AsyncFuture_TaskState_SET,
			memory_order_acq_rel, memory_order_acquire
		)
	)
		return true;

	#if BUILD_SAFE
	if (want == AsyncFuture_TaskState_SET)
		PANIC("already set");
	#endif

	return false;
}
