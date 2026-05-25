#ifndef AsyncFuture_PTRTAG
	#define AsyncFuture_PTRTAG PTRTAG
#endif

bool AsyncFuture_isnull(AsyncFuture this) {
	return this.value == nullptr;
}

#define AsyncFuture_NULL LITERAL(AsyncFuture,.value=nullptr)

#if AsyncFuture_PTRTAG
	typedef utag AsyncFutureMetadata;
	#define AsyncFuture_METADATA
	#define AsyncFuture_TOMETA(p) (p)
	#define AsyncFuture_TODATA(T, p) ((T*)p)

	Ptr AsyncFuture_data(AsyncFuture this) {
		return ptrstrip(this.value);
	}

	AsyncFutureMetadata AsyncFuture_metadata(AsyncFuture this) {
		return ptrread(this.value);
	}

	AsyncFuture AsyncFuture_upcast(void *this, AsyncFutureMetadata metadata) {
		return (AsyncFuture){.value=ptrtag(this,metadata)};
	}

	AsyncFuture AsyncFuture_setmeta(void *this, AsyncFutureMetadata metadata) {
		return AsyncFuture_upcast(this, metadata);
	}

#else
	typedef u16 AsyncFutureMetadata;
	#define AsyncFuture_METADATA AsyncFutureMetadata metadata;
	#define AsyncFuture_TOMETA(p) (&(p)->metadata)
	#define AsyncFuture_TODATA(T, p) ((T*)((ubyte)p - offsetof(T, metadata)))

	Ptr AsyncFuture_data(AsyncFuture this) {
		return this.value;
	}

	AsyncFutureMetadata AsyncFuture_metadata(AsyncFuture this) {
		return *((AsyncFutureMetadata*)this.value);
	}

	AsyncFuture AsyncFuture_setmeta(AsyncFutureMetadata *this, AsyncFutureMetadata metadata) {
		*this = metadata;
		return (AsyncFuture){.value=this};
	}

	AsyncFuture AsyncFuture_upcast(
		AsyncFutureMetadata *this, AsyncFutureMetadata metadata_unused
	) {
		return (AsyncFuture){.value=this};
	}
#endif

typedef enum : u8 {
	AsyncFutureType_RESUME,
	AsyncFutureType_COUNT_RESUME,

	AsyncFutureType_WAKE,
	AsyncFutureType_COUNT_WAKE,

	AsyncFutureType_CALLBACK,
	AsyncFutureType_COUNT_CALLBACK,
} AsyncFutureType;

AsyncFutureType AsyncFuture_type(AsyncFuture this) {
	return (AsyncFutureType)AsyncFuture_metadata(this);
}

enum {
	AsyncFutureState_NONE,
	AsyncFutureState_WAITING,
	AsyncFutureState_RESOLVED,
};

#include "AsyncFuture_Resume.h"
#include "AsyncFuture_Wake.h"
#include "AsyncFuture_Callback.h"

AsyncResult AsyncFuture_callback(AsyncFuture this) {
	Ptr data = AsyncFuture_data(this);

	switch (AsyncFuture_type(this)) {
		case AsyncFutureType_RESUME:
			return AsyncFuture_Resume_callback(data);
		case AsyncFutureType_COUNT_RESUME:
			return AsyncFuture_CountResume_callback(data);
		case AsyncFutureType_WAKE:
			return AsyncFuture_Wake_callback(data);
		case AsyncFutureType_COUNT_WAKE:
			return AsyncFuture_CountWake_callback(data);
		case AsyncFutureType_CALLBACK:
			return AsyncFuture_Callback_callback(data);
		case AsyncFutureType_COUNT_CALLBACK:
			return AsyncFuture_CountCallback_callback(data);

		default:;
	}

	UNREACHABLE;
}
