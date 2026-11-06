#ifndef AsyncTask_PTRTAG
	#define AsyncTask_PTRTAG PTRTAG
#endif

#if AsyncTask_PTRTAG
	typedef utag AsyncTaskState;
	constexpr auto AsyncTaskState_width = utag_width;
	constexpr AsyncTaskState AsyncTaskState_max = utag_max;

#else
	typedef u32 AsyncTaskState;
	constexpr auto AsyncTaskState_width = 32;
	constexpr AsyncTaskState AsyncTaskState_max = u32_max;

#endif

typedef usize AsyncTaskContext;
constexpr auto AsyncTaskContext_width = usize_width - AsyncTaskState_width;
constexpr AsyncTaskContext AsyncTaskContext_max = ((1ull << AsyncTaskContext_width) - 1);

usize AsyncTask_makeargs(AsyncTaskState state, AsyncTaskContext context) {
	#if AsyncTask_SAFE
		ASSERT(state <= AsyncTaskState_max);
		ASSERT(context <= AsyncTaskContext_max);
	#endif

	return (usize)(
		((usize)context << AsyncTaskState_width) |
		(usize)state
	);
}

AsyncTaskState AsyncTask_getstate(usize args) {
	return (AsyncTaskState)(args & AsyncTaskState_max);
}

AsyncTaskContext AsyncTask_getcontext(usize args) {
	return (AsyncTaskContext)((args >> AsyncTaskState_width) & AsyncTaskContext_max);
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

typedef AsyncRes (*AsyncFn)(
	Ptr data, AsyncRT *rt, usize args, AsyncTask *out_task
);

AsyncRes AsyncTask_call(AsyncTask this, AsyncRT *rt, AsyncTaskContext ctx, AsyncTask *out_task) {
	auto data = AsyncTask_data(this);
	auto args = AsyncTask_makeargs(AsyncTask_state(this), ctx);

	return (*(AsyncFn*)data)(data, rt, args, out_task);
}
