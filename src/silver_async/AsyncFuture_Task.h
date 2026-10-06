typedef union {
	alignas(AsyncFuture_minalign) char align__;
	struct {
		AsyncTask task;
		_Atomic u32 state;
	};
} AsyncFuture_Task;

AsyncFuture AsyncFuture_Task_upcast(AsyncFuture_Task *this) {
	return AsyncFuture_upcast(this, AsyncFutureClass_Task);
}

AsyncFuture AsyncFuture_Task_init(AsyncFuture_Task *this) {
	this->state = AsyncFutureState_None;
	return AsyncFuture_Task_upcast(this);
}

AsyncIntent AsyncFuture_Task_resolve(AsyncFuture_Task *this, AsyncResult *result) {
	if (
		atom_exchg(&this->state, AsyncFutureState_Resolved, atom_sync)
			== AsyncFutureState_Ready
	) {
		result->task = this->task;
		return AsyncIntent_Resume;
	}

	return AsyncIntent_Yield;
}

bool AsyncFuture_Task_set(AsyncFuture_Task *this, AsyncTask task) {
	u32 want = AsyncFutureState_None;
	if (atom_cmpx(&this->state, &want, AsyncFutureState_Ready, atom_sync, atom_acq)) {
		this->task = task;
		return true;
	}

	#if LIBSILVER_SAFE
		if (want == AsyncFutureState_Ready)
			PANIC("already set");
	#endif

	return false;
}

typedef union {
	alignas(AsyncFuture_minalign) char align__;
	struct {
		AsyncTask task;
		_Atomic u32_lf state;
		_Atomic u32_lf count;
	};
} AsyncFuture_TaskCount;

AsyncFuture AsyncFuture_TaskCount_upcast(AsyncFuture_TaskCount *this) {
	return AsyncFuture_upcast(this, AsyncFutureClass_TaskCount);
}

AsyncFuture AsyncFuture_TaskCount_init(AsyncFuture_TaskCount *this, u32 count) {
	this->state = AsyncFutureState_None;
	this->count = count;
	return AsyncFuture_TaskCount_upcast(this);
}

AsyncIntent AsyncFuture_TaskCount_resolve(AsyncFuture_TaskCount *this, AsyncResult *result) {
	if (atom_sub(&this->count, 1, atom_sync) != 1)
		return AsyncIntent_Yield;

	if (
		// TODO validate atom_sync memory order
		atom_exchg(&this->state, AsyncFutureState_Resolved, atom_sync)
			== AsyncFutureState_Ready
	) {
		result->task = this->task;
		return AsyncIntent_Resume;
	}

	return AsyncIntent_Yield;
}

bool AsyncFuture_TaskCount_set(AsyncFuture_TaskCount *this, AsyncTask task) {
	u32 want = AsyncFutureState_None;
	if (atom_cmpx(&this->state, &want, AsyncFutureState_Ready, atom_sync, atom_acq)) {
		this->task = task;
		return true;
	}

	#if LIBSILVER_SAFE
		if (want == AsyncFutureState_Ready)
			PANIC("already set");
	#endif

	return false;
}
