#ifndef AsyncTask_PTRTAG
	#define AsyncTask_PTRTAG PTRTAG
#endif

#if AsyncTask_PTRTAG
	typedef utag AsyncTaskState;

	#define AsyncTaskState_BITS PTRTAG_BITS
	#define AsyncTaskState_MAX PTRTAG_MAX

#else
	typedef u32 AsyncTaskState;

	#define AsyncTaskState_BITS 32
	#define AsyncTaskState_MAX UINT32_MAX

#endif

typedef u64 AsyncTaskArgs;
typedef u64 AsyncTaskContext;

#define AsyncTaskArgs_BITS 64
#define AsyncTaskContext_BITS (AsyncTaskArgs_BITS - AsyncTaskState_BITS)
#define AsyncTaskContext_MAX ((1ull << AsyncTaskContext_BITS) - 1)

AsyncTaskArgs AsyncTaskArgs_create(AsyncTaskContext context, AsyncTaskState state) {
	return (AsyncTaskArgs)(
		((AsyncTaskArgs)context << AsyncTaskState_BITS) |
		(AsyncTaskArgs)state
	);
}

AsyncTaskState AsyncTaskArgs_state(AsyncTaskArgs args) {
	return (AsyncTaskState)(args & AsyncTaskState_MAX);
}

AsyncTaskContext AsyncTaskArgs_context(AsyncTaskArgs args) {
	return (AsyncTaskContext)(args >> AsyncTaskState_BITS);
}


#if AsyncTask_PTRTAG
	struct AsyncTask {
		Ptr value;
	};

	Ptr AsyncTask_data(AsyncTask this) {
		return ptrstrip(this.value);
	}

	AsyncTaskState AsyncTask_state(AsyncTask this) {
		return ptrread(this.value);
	}

	AsyncTask AsyncTask_upcast(Ptr data, AsyncTaskState state) {
		return (AsyncTask){.value=ptrtag(data, state)};
	}

	#define AsyncTask_NULL LITERAL(AsyncTask, .value=nullptr)

	bool AsyncTask_isnull(AsyncTask this) {
		return this.value == nullptr;
	}

#else
	struct AsyncTask {
		Ptr data;
		AsyncTaskState state;
	};

	Ptr AsyncTask_data(AsyncTask this) {
		return this.data;
	}

	AsyncTaskState AsyncTask_state(AsyncTask this) {
		return this.state;
	}

	AsyncTask AsyncTask_upcast(Ptr data, AsyncTaskState state) {
		return (AsyncTask){.state=state,.data=data};
	}

	#define AsyncTask_NULL LITERAL(AsyncTask, .data=nullptr)

	bool AsyncTask_isnull(AsyncTask this) {
		return this.data == nullptr;
	}

#endif

typedef union {
	AsyncTask out_task;
} AsyncTaskIO;

typedef AsyncIntent (*AsyncFn)(
	AsyncRT *rt, Ptr data, AsyncTaskArgs args, AsyncTaskIO *io
);

AsyncIntent AsyncTask_call(AsyncTask this, AsyncTaskContext ctx, AsyncRT *rt, AsyncTaskIO *io) {
	auto data = AsyncTask_data(this);
	auto args = AsyncTaskArgs_create(ctx, AsyncTask_state(this));

	return (*(AsyncFn*)data)(rt, data, args, io);
}
