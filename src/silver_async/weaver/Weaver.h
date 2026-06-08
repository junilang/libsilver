enum : umtx {
	Weaver_LOCK_NONE,
	Weaver_LOCK_JOIN,
	Weaver_LOCK_DOWN
};

#if Weaver_CACHE_ALIGN
	typedef struct {
		// read only section
		alignas(Weaver_CACHE_ALIGN) struct {
			AsyncRT iface;
			Allocator alc;
			u16 threads_size;
		};

		WeaverQueue queue[2];

		alignas(Weaver_CACHE_ALIGN) WeaverQinfo qinfo;
		alignas(Weaver_CACHE_ALIGN) WeaverQinfo mqinfo;

		WeaverThread threads[];
	} Weaver;


#else
	typedef struct {
		AsyncRT iface;
		Allocator alc;

		WeaverQueue queue[2];

		_Atomic WeaverQinfo qinfo;
		_Atomic WeaverQinfo mqinfo;

		_Atomic umtx lock;
		_Atomic u32 threads_sync;

		u16 threads_size;

		WeaverThread threads[];
	} Weaver;
#endif

usize ZZWeaver_allocsize(u16 threads_size) {
	return offsetof(Weaver, threads) + (sizeof(WeaverThread) * threads_size);
}

AsyncRT *Weaver_upcast(Weaver *this) {
	return &this->iface;
}

void Weaver_submit(Ptr vthis, const AsyncTask *tasks, usize tasks_size);

void Weaver_init(
	Weaver *this, u16 threads_size, usize queue_capacity, Allocator queue_alc
) {
	if (threads_size > Weaver_MAX_THREADS)
		PANIC("max threads limit");

	if (queue_capacity < 2)
		PANIC("queue capacity must be at least 2")


	// setup interface
	this->iface.submit = &Weaver_submit;

	this->queue_alc = queue_alc;
	this->threads_size = threads_size;

	WeaverQueue *qs[2] = {
		Allocator_new(this->queue_alc, ZZWeaverQueue_allocsize(queue_capacity)),
		Allocator_new(this->queue_alc, ZZWeaverQueue_allocsize(queue_capacity))
	};

	qs[0]->capacity = queue_capacity;
	qs[0]->size = 0;

	qs[1]->capacity = queue_capacity;
	qs[1]->size = 0;

	this->queue[0] = qs[0];
	this->queue[1] = qs[1];

	this->qinfo = 0;
	this->mqinfo = 1;

	this->threads_sync = 0;
	this->lock = Weaver_LOCK_NONE;
}

void Weaver_deinit(Weaver *this) {
	if (this->queue[0]) Allocator_delete(this->queue_alc, this->queue[0]);
	if (this->queue[1]) Allocator_delete(this->queue_alc, this->queue[1]);
}

Weaver *Weaver_create(Allocator alc, u16 threads_size, usize queue_capacity) {
	Weaver *this = Allocator_new(alc, ZZWeaver_allocsize(threads_size));
	Weaver_init(this, threads_size, queue_capacity, alc);
	return this;
}

void Weaver_destroy(Weaver *this) {
	// assumes allocator of this and queue_alc are equal
	Allocator alc = this->queue_alc;
	Weaver_deinit(this);
	Allocator_delete(alc, this);
}
