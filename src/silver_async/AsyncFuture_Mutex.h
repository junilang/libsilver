AsyncWaitRes AsyncFuture_Mutex_ZZwait(_Atomic u32_lf *state) {
	if (atom_get(state, atom_acq) == AsyncFutureState_Resolved)
		return AsyncWaitRes_Ok;

	u32 want = AsyncFutureState_None;
	atom_cmpx(state, &want, AsyncFutureState_Ready, atom_sync, atom_acq);

	if (want == AsyncFutureState_Resolved)
		return AsyncWaitRes_Ok;

	do {
		auto res = Async_wait32(state, AsyncFutureState_Ready, 0);
		switch (res) {
			default:
				return res;

			// if there was a mismatch, the future must already be Resolved
			case AsyncWaitRes_Mismatch:;
			// if thread was woken up it must be because the future has been Resolved
			case AsyncWaitRes_Ok:;
				return AsyncWaitRes_Ok;

			case AsyncWaitRes_Interrupt:;
		}
	} while (
		atom_get(state, atom_lazy) == AsyncFutureState_Ready
	);

	return AsyncWaitRes_Ok;
}

AsyncWaitRes AsyncFuture_Mutex_ZZresolve(_Atomic u32_lf *state) {
	if (
		atom_exchg(state, AsyncFutureState_Resolved, atom_sync)
			== AsyncFutureState_Ready
	)
		return Async_wake32(state, Async_wakeall);

	return AsyncWaitRes_Ok;
}

typedef union {
	_Alignas(AsyncFuture_minalign) char align__;
	_Atomic u32_lf state;
} AsyncFuture_Mutex;

AsyncFuture AsyncFuture_Mutex_upcast(AsyncFuture_Mutex *this) {
	return AsyncFuture_upcast(this, AsyncFutureClass_Mutex);
}

AsyncFuture AsyncFuture_Mutex_init(AsyncFuture_Mutex *this) {
	this->state = AsyncFutureState_None;
	return AsyncFuture_Mutex_upcast(this);
}

AsyncIntent AsyncFuture_Mutex_resolve(AsyncFuture_Mutex *this) {
	if (AsyncFuture_Mutex_ZZresolve(&this->state))
		return AsyncIntent_Error;
	return AsyncIntent_Yield;
}

AsyncWaitRes AsyncFuture_Mutex_wait(AsyncFuture_Mutex *this) {
	return AsyncFuture_Mutex_ZZwait(&this->state);
}

typedef union {
	_Alignas(AsyncFuture_minalign) char align__;
	struct {
		_Atomic u32_lf state;
		_Atomic u32_lf count;
	};
} AsyncFuture_MutexCount;

AsyncFuture AsyncFuture_MutexCount_upcast(AsyncFuture_MutexCount *this) {
	return AsyncFuture_upcast(this, AsyncFutureClass_MutexCount);
}

AsyncFuture AsyncFuture_MutexCount_init(AsyncFuture_MutexCount *this, u32 count) {
	this->state = AsyncFutureState_None;
	this->count = count;
	return AsyncFuture_MutexCount_upcast(this);
}

AsyncIntent AsyncFuture_MutexCount_resolve(AsyncFuture_MutexCount *this) {
	if (atom_sub(&this->count, 1, atom_sync) == 1) {
		if (AsyncFuture_Mutex_ZZresolve(&this->state))
			return AsyncIntent_Error;
	}

	return AsyncIntent_Yield;
}

AsyncWaitRes AsyncFuture_MutexCount_wait(AsyncFuture_MutexCount *this) {
	return AsyncFuture_Mutex_ZZwait(&this->state);
}
