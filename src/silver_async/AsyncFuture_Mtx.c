enum : umtx {
	AsyncFuture_MtxState_NONE,
	AsyncFuture_MtxState_WAITING,
	AsyncFuture_MtxState_RESOLVED
};

void ZZAsyncFuture_Mtx_wait(_Atomic umtx *state) {
	if (
		atomic_load_explicit(state, memory_order_acquire)
			== AsyncFuture_MtxState_RESOLVED
	)
		return;


	umtx want = AsyncFuture_MtxState_NONE;
	atomic_compare_exchange_strong_explicit(
		state, &want, AsyncFuture_MtxState_WAITING,
		memory_order_acq_rel, memory_order_acquire
	);

	if (want != AsyncFuture_MtxState_RESOLVED) {
		do {
			switch(umtx_wait(state, AsyncFuture_MtxState_WAITING)) {
				case umtxResult_MISMATCH:
					return;
				default:;
			}
		} while (
			atomic_load_explicit(state, memory_order_relaxed)
				== AsyncFuture_MtxState_WAITING
		);
	}
}

void ZZAsyncFuture_Mtx_resolve(_Atomic umtx *state) {
	if (
		atomic_exchange_explicit(state, AsyncFuture_MtxState_RESOLVED, memory_order_acq_rel)
			== AsyncFuture_MtxState_WAITING
	) {
		umtxResult res = umtx_wake_all(state);
		switch (res) {
			default:;
		}
	}
}

typedef union {
	alignas(AsyncFuture_ALIGN) char align__;
	struct {
		_Atomic umtx state;
	};
} AsyncFuture_Mtx;

AsyncFuture AsyncFuture_Mtx_upcast(AsyncFuture_Mtx *this) {
	return AsyncFuture_tag(this, AsyncFutureClass_MTX);
}

AsyncFuture AsyncFuture_Mtx_init(AsyncFuture_Mtx *this) {
	this->state = AsyncFuture_MtxState_NONE;
	return AsyncFuture_Mtx_upcast(this);
}

AsyncIntent AsyncFuture_Mtx_resolve(AsyncFuture_Mtx *this, AsyncResult *result) {
	ZZAsyncFuture_Mtx_resolve(&this->state);
	return AsyncIntent_YIELD;
}

void AsyncFuture_Mtx_wait(AsyncFuture_Mtx *this) {
	ZZAsyncFuture_Mtx_wait(&this->state);
}


typedef union {
	alignas(AsyncFuture_ALIGN) char align__;
	struct {
		_Atomic umtx state;
		_Atomic u32 count;
	};
} AsyncFuture_MtxCount;

AsyncFuture AsyncFuture_MtxCount_upcast(AsyncFuture_MtxCount *this) {
	return AsyncFuture_tag(this, AsyncFutureClass_MTX_COUNT);
}

AsyncFuture AsyncFuture_MtxCount_init(AsyncFuture_MtxCount *this, u32 count) {
	this->state = AsyncFuture_MtxState_NONE;
	this->count = count;
	return AsyncFuture_MtxCount_upcast(this);
}

AsyncIntent AsyncFuture_MtxCount_resolve(AsyncFuture_MtxCount *this, AsyncResult *result) {
	if (atomic_fetch_sub_explicit(&this->count, 1, memory_order_acq_rel) == 1)
		ZZAsyncFuture_Mtx_resolve(&this->state);

	return AsyncIntent_YIELD;
}

void AsyncFuture_MtxCount_wait(AsyncFuture_MtxCount *this) {
	ZZAsyncFuture_Mtx_wait(&this->state);
}
