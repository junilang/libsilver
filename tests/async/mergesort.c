#define async ZZmergesort
	async_state(
		ZZmergesort_Task *children;
		int *src;
		int *dst;
		usize size;
	)

	async_labels(fork, join)

	async_begin {
		if (this->size > 4) {
			this->children = AsyncRT_alc
		}
	}


	async_fork(fork, 4) {
		PRINTBP(128, Stdout, "context=",asynx_context__,"\n");
		atom_add(this->target, 1, atom_sync);
	}

	async_join(join) {
		async_return;
	}

	async_end
#undef async

constexpr u32 ZZthreads = 4;
_Alignas(Weaver) ubyte ZZweaver_[Weaver_ALLOCSIZE(ZZthreads)];

int ZZentry(SilverTestContext *ctx) {
	Ptr heap;
	usize heap_size;
	{
		AlcReq req = {
			.intent = AlcIntent_New,
			.size = (1024 * 1024) * 128,
		};
		AlcPtr ptr = Alc_invoke(OsAlc, &req, nullptr, nullptr);
		AlcRes res = AlcPtr_get(ptr);
		if (res) return 1;

		heap = (Ptr)ptr;
		heap_size = req.size;
	}

	auto const heap_alc_ = StaticAlc_init(heap, heap_size);
	auto const heap_alc = StaticAlc_upcast(heap_alc_);

	Weaver *weaver;
	{
		AlcPtr ptr = Weaver_create(heap_alc, 4, 8);
		auto const res = AlcPtr_get(ptr);
		if (res) return 2;

		weaver = (Ptr)ptr;
	}

	constexpr usize array_size = 10240000;
	i32 *array;
	{
		AlcReq req = {
			.intent = AlcIntent_New,
			.align = _Alignof(i32),
			.size = sizeof(i32) * array_size
		};

		AlcPtr ptr = Alc_invoke(heap_alc, &req, nullptr, nullptr);
		auto const res = AlcPtr_get(ptr);
		if (res) return 3;

		array = (Ptr)ptr;
	}

	{ // seed array
		u64 state = 0x321321321321343ull;
		auto it = array;
		auto const end = array + array_size;

		for (; it < end; it++) {
			*it = splitmix64(&state) % 10'000;
		}
	}

	if (array_size < 200) {
		PRINTBP(32, Stdout, array[0]);
		for (usize i = 1; i < array_size; i++) {
			PRINTBP(32, Stdout, ", ", array[i]);
		}
		PRINTP(Stdout, "\n");
	}


	AsyncRes ares;

	ares = Weaver_boot(weaver);
	if (ares) goto async_error;

	AsyncFuture future;
	AsyncFuture_init(&future, 1);

	AsyncFuture_wait(&future, nullptr);

	ares = Weaver_join(weaver);
	if (ares) goto async_error;

	{
		auto const res = Alc_delete(OsAlc, heap);
		if (res) return 99;
	}

	if (0) async_error: {
		return 666;
	}

	return 0;
}
