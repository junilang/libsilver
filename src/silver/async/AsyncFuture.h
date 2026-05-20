#ifndef AsyncFuture_PTRTAG
	#define AsyncFuture_PTRTAG PTRTAG
#endif

bool AsyncFuture_isnull(AsyncFuture this) {
	return this.value == nullptr;
}

#define AsyncFuture_NULL ((AsyncFuture){.value=nullptr})

#if AsyncFuture_PTRTAG
	typedef utag AsyncFutureMetadata;
	#define AsyncFuture_DATAHEADER

	Ptr AsyncFuture_data(AsyncFuture this) {
		return ptrstrip(this.value);
	}

	AsyncFutureMetadata AsyncFuture_metadata(AsyncFuture this) {
		return ptrread(this.value);
	}

#else
	typedef u32 AsyncFutureMetadata;
	#define AsyncFuture_DATAHEADER AsyncFutureMetadata metadata;

	Ptr AsyncFuture_data(AsyncFuture this) {
		return this.value;
	}

	AsyncFutureMetadata AsyncFuture_metadata(AsyncFuture this) {
		return *((AsyncFutureMetadata*)this.value);
	}
#endif

typedef enum : u8 {
	AsyncFutureType_RESUME
} AsyncFutureType;

AsyncFutureType AsyncFuture_type(AsyncFuture this) {
	return (AsyncFutureType)AsyncFuture_metadata(this);
}


typedef enum : usize {
	AsyncFutureState_RUNNING,
	AsyncFutureState_FINISHED,
} AsyncFutureState;

typedef struct {
	AsyncFuture_DATAHEADER
	pthread_mutex_t lock;
	union {
		AsyncFutureState state;
		AsyncTask resume;
	};
} AsyncFuture_Resume;

AsyncResult AsyncFuture_Resume_callback(AsyncFuture_Resume *this) {
	AsyncResult result;

	pthread_mutex_lock(&this->lock);

	switch (this->state) {
		case AsyncFutureState_RUNNING:
			this->state = AsyncFutureState_FINISHED;
			result = AsyncResult_SUSPEND;
			break;
		case AsyncFutureState_FINISHED:
			result = AsyncResult_PANIC;
			break;
		default:
			result.next = this->resume;
			break;
	}

	pthread_mutex_unlock(&this->lock);

	return result;
}

AsyncResult AsyncFuture_callback(AsyncFuture this) {
	Ptr data = AsyncFuture_data(this);

	switch (AsyncFuture_type(this)) {
		case AsyncFutureType_RESUME:
			return AsyncFuture_Resume_callback(data);
	}

	UNREACHABLE;
}
