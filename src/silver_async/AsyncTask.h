#ifndef AsyncTask_PTRTAG
	#define AsyncTask_PTRTAG PTRTAG
#endif

typedef STRUCTDECL(AsyncTaskData);

#if AsyncTask_PTRTAG
	#define AsyncTaskMetadata_BITS PTRTAG_BITS
	#define AsyncTaskMetadata_MAX PTRTAG_MAX

	typedef utag AsyncTaskMetadata;

	AsyncTaskMetadata AsyncTask_metadata(AsyncTask this) {
		return ptrread(this.value);
	}

	AsyncTaskData *AsyncTask_data(AsyncTask this) {
		return ptrstrip(this.value);
	}

	AsyncTask AsyncTask_setmeta(AsyncTaskData *this, AsyncTaskMetadata metadata) {
		return (AsyncTask){.value=ptrtag(this, metadata)};
	}

#else
	typedef u32 AsyncTaskMetadata;

	#define AsyncTaskMetadata__BITS (sizeof(AsyncTaskMetadata) * CHAR_BIT)
	#define AsyncTaskMetadata__MAX UINT32_MAX


	AsyncTaskMetadata AsyncTask_metadata(AsyncTask this) {
		return ((AsyncTaskData*)this)->metadata;
	}

	AsyncTaskData *AsyncTask_data(AsyncTask this) {
		return this.value;
	}

	AsyncTask AsyncTask_setmeta(AsyncTaskData *this, AsyncTaskMetadata metadata) {
		this->metadata = metadata;
		return (AsyncTask){.value=this};
	}

#endif

bool AsyncTask_isnull(AsyncTask task) {
	return task.value == nullptr;
}

#define AsyncTask_NULL ((AsyncTask){.value=nullptr})

typedef AsyncTaskMetadata AsyncTaskState;

#define AsyncTaskFlag_BITS 2
#define AsyncTaskState_BITS (AsyncTaskMetadata_BITS - AsyncTaskFlag_BITS)
#define AsyncTaskState_MASK ((AsyncTaskMetadata)(AsyncTaskMetadata_MAX >> AsyncTaskFlag_BITS))

enum {
	AsyncTaskFlag_BIT_RESUMING = AsyncTaskState_BITS,
	AsyncTaskFlag_BIT_SUSPEND
};

AsyncTaskState AsyncTask_state(AsyncTask this) {
	return AsyncTask_metadata(this) & AsyncTaskState_MASK;
}

AsyncTaskMetadata AsyncTask_flags(AsyncTask this) {
	return (AsyncTaskMetadata)(
		AsyncTask_metadata(this) & (~AsyncTaskState_MASK)
	);
}

typedef AsyncResult (*AsyncFn)(AsyncRT rt, AsyncTask task);

struct AsyncTaskData {
#if !AsyncTask_PTRTAG
	AsyncTaskMetadata metadata;
#endif
	AsyncFn entry;
	union {
		AsyncFuture future;
		AsyncTask resume;
	};
};

AsyncTaskMetadata AsyncTaskMetadata_create(AsyncTaskMetadata flags, AsyncTaskState state) {
	return flags | state;
}

void AsyncTask_xprint(AsyncTask this, OutStream os) {
	if (AsyncTask_isnull(this)) {
		PRINT(os, "AsyncTask_NULL");
		return;
	}

	auto const data = AsyncTask_data(this);

	PRINT(os, "AsyncTask(",(Ptr)data,", ");
	if (AsyncTask_flags(this) & FLAG(AsyncTaskFlag, RESUMING)) {
		PRINT(os,"R, ");
	}

	PRINT(os, "S",AsyncTask_state(this),")");
}

void AsyncTask_print(const AsyncTask *this, OutStream os) {
	AsyncTask_xprint(*this, os);
}

IPrintable_GENERATE(AsyncTask, const AsyncTask*)
