typedef union {
	usize raw_value;
	Ptr value;
} AsyncFuture;

typedef enum : u8 {
	AsyncFutureClass_Mutex,
	AsyncFutureClass_MutexCount,
	AsyncFutureClass_Task,
	AsyncFutureClass_TaskCount
} AsyncFutureClass;

enum {
	AsyncFutureState_None,
	AsyncFutureState_Ready,
	AsyncFutureState_Resolved
};

constexpr usize AsyncFuture_tagmask = 0b11;
constexpr ualign AsyncFuture_minalign = AsyncFuture_tagmask + 1;

AsyncFutureClass AsyncFuture_class(AsyncFuture this) {
	return (AsyncFutureClass)(this.raw_value & AsyncFuture_tagmask);
}

Ptr AsyncFuture_data(AsyncFuture this) {
	return (Ptr)(this.raw_value & (~AsyncFuture_tagmask));
}

AsyncFuture AsyncFuture_upcast(Ptr data, AsyncFutureClass class) {
	return (AsyncFuture){ .raw_value = (usize)data | class };
}

constexpr AsyncFuture AsyncFuture_null = { .value = nullptr };

bool AsyncFuture_isnull(AsyncFuture this) {
	return this.value == nullptr;
}
