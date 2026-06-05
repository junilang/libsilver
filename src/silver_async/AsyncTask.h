#ifndef AsyncTask_PTRTAG
	#define AsyncTask_PTRTAG PTRTAG
#endif

typedef STRUCTDECL(AsyncTaskData);

#if AsyncTask_PTRTAG
	typedef utag AsyncTaskState;

	#define AsyncTaskState_BITS PTRTAG_BITS
	#define AsyncTaskState_MAX PTRTAG_MAX

#else
	typedef u64 AsyncTaskState;

	#define AsyncTaskState_BITS 64
	#define AsyncTaskState_MAX UINT64_MAX

#endif

typedef AsyncIntent (*AsyncFn)(
	AsyncRT *rt, Ptr data, AsyncTaskState state, AsyncResult *result
);


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
		AsyncTaskState state;
		Ptr data;
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

AsyncIntent AsyncTask_call(AsyncTask this, AsyncRT *rt, AsyncResult *result) {
	auto data = AsyncTask_data(this);
	auto state = AsyncTask_state(this);

	return (*(AsyncFn*)data)(rt, data, state, result);
}
