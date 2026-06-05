typedef struct {
	Ptr value;
} AsyncFuture;

#define AsyncFutureClass__BITS 2
#define AsyncFuture_ALIGN (1ull << AsyncFutureClass__BITS)

typedef enum : u8 {
	AsyncFutureClass_MTX,
	AsyncFutureClass_MTX_COUNT,
	AsyncFutureClass_TASK,
	AsyncFutureClass_TASK_COUNT,
	AsyncFutureClass__MAX
} AsyncFutureClass;

static_assert(AsyncFutureClass__MAX <= AsyncFuture_ALIGN);

AsyncFutureClass AsyncFuture_class(AsyncFuture this) {
	return (AsyncFutureClass)lptrread(this.value, AsyncFutureClass__BITS);
}

Ptr AsyncFuture_data(AsyncFuture this) {
	return lptrstrip(this.value, AsyncFutureClass__BITS);
}

AsyncFuture AsyncFuture_tag(Ptr this, AsyncFutureClass class) {
	return (AsyncFuture){.value=lptrtag(this, AsyncFutureClass__BITS, class)};
}

AsyncIntent AsyncFuture_resolve(AsyncFuture this, AsyncResult *result);

#define AsyncFuture_NULL ((AsyncFuture){.value=nullptr})

bool AsyncFuture_isnull(AsyncFuture this) {
	return this.value == nullptr;
}
