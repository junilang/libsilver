#ifndef AsyncRT_PTRTAG
	#define AsyncRT_PTRTAG PTRTAG
#endif

typedef struct {
	void (*submit)(Ptr this, const AsyncTask *tasks, usize tasks_size);
} IAsyncRT;

#if AsyncRT_PTRTAG
	struct AsyncRT {
		Ptr value;
	};

	enum {
		IAsyncRT_DummyAsync_ID,
		IAsyncRT_Weaver_ID,
		IAsyncRT_KNOWN
	};

	INTERFACE_REGISTRY(IAsyncRT, u8, 8)

	Ptr AsyncRT_this(AsyncRT this) { return ptrstrip(this.value); }
	const IAsyncRT *AsyncRT_iface(AsyncRT this) {
		return &IAsyncRT__registry[ptrread(this.value)];
	}

#endif

#include "AsyncRT_meta.h"

void AsyncRT_submit(AsyncRT this, const AsyncTask *tasks, usize tasks_size) {
	AsyncRT_iface(this)->submit(AsyncRT_this(this), tasks, tasks_size);
}

void AsyncFuture_resolve(AsyncFuture this, AsyncRT rt) {
	auto result = AsyncFuture_callback(this);
	switch (result.intent) {
		case AsyncIntent_YIELD:
		case AsyncIntent_FINISH:
			return;
		default:;
	}

	AsyncRT_submit(rt, &result.next, 1);
}
