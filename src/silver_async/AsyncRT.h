#ifndef AsyncRT_PTRTAG
	#define AsyncRT_PTRTAG PTRTAG
#endif

typedef struct {
	void (*submit)(Ptr this, const AsyncTask *tasks, usize tasks_size);
	void (*resolve)(Ptr this, const AsyncFuture *futures, usize futures_size);
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

void AsyncRT_resolve(AsyncRT this, const AsyncFuture *futures, usize futures_size) {
	AsyncRT_iface(this)->resolve(AsyncRT_this(this), futures, futures_size);
}
