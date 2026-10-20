#ifndef AsyncTask_PTRTAG
	#define AsyncTask_PTRTAG PTRTAG
#endif

#if AsyncTask_PTRTAG
	typedef utag AsyncTaskState;
	constexpr u8 AsyncTaskState_width = utag_width;
	constexpr AsyncTaskState AsyncTaskState_max = utag_max;

#else
	typedef u32 AsyncTaskState;
	constexpr u8 AsyncTaskState_width = 32;
	constexpr AsyncTaskState AsyncTaskState_max = u32_max;

#endif

typedef usize AsyncTaskArgs;
constexpr u8 AsyncTaskArgs_width = usize_width;

typedef usize AsyncTaskContext;
constexpr u8 AsyncTaskContext_width = AsyncTaskArgs_width - AsyncTaskState_width;
constexpr AsyncTaskContext AsyncTaskContext_max = ((1ull << AsyncTaskContext_width) - 1);

AsyncTaskArgs AsyncTaskArgs_create(AsyncTaskContext context, AsyncTaskState state) {
	return (AsyncTaskArgs)(
		((AsyncTaskArgs)context << AsyncTaskState_width) |
		(AsyncTaskArgs)state
	);
}

AsyncTaskState AsyncTaskArgs_state(AsyncTaskArgs args) {
	return (AsyncTaskState)(args & AsyncTaskState_max);
}

AsyncTaskContext AsyncTaskArgs_context(AsyncTaskArgs args) {
	return (AsyncTaskContext)(args & AsyncTaskContext_max);
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
		return (AsyncTask){ .value = ptrtag(data, state) };
	}

	constexpr AsyncTask AsyncTask_null = { .value = nullptr };

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
		return (AsyncTask){ .data = data, .state = state };
	}

	constexpr AsyncTask AsyncTask_null = { .data = nullptr };

	bool AsyncTask_isnull(AsyncTask this) {
		return this.data = nullptr;
	}

#endif

struct AsyncResult {
	union {
		AsyncTask task;
		AsyncError error;
	};
};

typedef AsyncIntent (*AsyncFn)(
	Ptr data, AsyncRT *rt, AsyncTaskArgs args, AsyncResult *result
);

AsyncIntent AsyncTask_call(AsyncTask this, AsyncRT *rt, AsyncTaskContext ctx, AsyncResult *result) {
	auto data = AsyncTask_data(this);
	auto args = AsyncTaskArgs_create(ctx, AsyncTask_state(this));

	return (*(AsyncFn*)data)(data, rt, args, result);
}
