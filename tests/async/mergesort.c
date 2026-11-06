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
			auto const alc = AsyncRT_alc(asynx_rt__, asynx_context__);
			AlcReq req = {
				.intent = AlcIntent_New,
				.size = sizeof(ZZmergesort_Task) * 2,
				.align = _Alignof(ZZmergesort_Task)
			};
			auto const ptr = Alc_invoke(alc, &req, nullptr, nullptr);
			asynx_error__.alc = AlcPtr_get(ptr);
			if (asynx_error__.alc) goto asynx_catch_alc__;
			this->children = (Ptr)ptr;
		}
	}

	async_fork(fork, 2) {
		auto src = this->src;
		auto dst = this->dst;
		auto size = this->size / 2;

		if (asynx_fork_id__ == 2) {
			src += size;
			dst += size;
			size = this->size - size;
		}

		switch (size) {
			case 2:
				if (src[0] > src[1]) {
					dst[0] = src[1];
					dst[1] = src[0];
				} else {
					dst[0] = src[0];
					dst[1] = src[1];
				}
			[[fallthrough]];
			case 1:
			case 0:
				async_join_to(join);
			default:
		}

		auto child = &this->children[asynx_fork_id__ - 1];
		*child = async_create(ZZmergesort,
			.src = src, .dst = dst, .size = size
		);

		async_call_join_to(join, child);
	}


	async_join(join) {
		if (this->children) {
			auto const alc = AsyncRT_alc(asynx_rt__, asynx_context__);
			asynx_error__.alc = Alc_delete(alc, this->children);
			if (asynx_error__.alc) goto asynx_catch_alc__;
		}

		auto ls = this->size / 2;
		auto left = this->dst;
		auto left_end = left + ls;
		auto right = left_end;
		auto right_end = left + this->size;

		auto dst = this->src;

		while (true) {
			if (*left <= *right) {
				*(dst++) = *(left++);
				if (left >= left_end) goto rest_right;
			} else {
				*(dst++) = *(right++);
				if (right >= right_end) goto rest_left;
			}
		}

		if (0) rest_left: {
			for (; left < left_end; left++) {
				*(dst++) = *left;
			}
		}

		if (0) rest_right: {
			for (; right < right_end; right++) {
				*(dst++) = *right;
			}
		}

		memcpy(this->dst, this->src, sizeof(int) * this->size);
	}

	async_end
#undef async

constexpr u32 ZZthreads = 4;
_Alignas(Weaver) ubyte ZZweaver_[Weaver_ALLOCSIZE(ZZthreads)];

TEST(ZZ) {
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

	AsyncAlc async_alc_;
	AsyncAlc_init(&async_alc_, StaticAlc_upcast(heap_alc_));
	auto const alc = AsyncAlc_upcast(&async_alc_);

	Weaver *weaver;
	{
		AlcPtr ptr = Weaver_create(alc, alc, 4, 8);
		auto const res = AlcPtr_get(ptr);
		if (res) return 2;

		weaver = (Ptr)ptr;
	}

	constexpr usize array_size = 1024;
	i32 *array;
	{
		AlcReq req = {
			.intent = AlcIntent_New,
			.align = _Alignof(i32),
			.size = sizeof(i32) * array_size * 2
		};

		AlcPtr ptr = Alc_invoke(alc, &req, nullptr, nullptr);
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
		PRINTBA(32, Stdout, array[0]);
		for (usize i = 1; i < array_size; i++) {
			PRINTBA(32, Stdout, ", ", array[i]);
		}
		PRINTA(Stdout, "\n");
	}

	auto dst = array + array_size;


	AsyncRes ares;

	ares = Weaver_boot(weaver);
	if (ares) goto async_error;

	AsyncFuture future = {};
	AsyncFuture_init(&future, 1);

	auto task = async_create_resolve(ZZmergesort, &future,
		.src = array,
		.dst = dst,
		.size = array_size
	);

	constexpr usize asynx_context__ = 0;
	async_spawn_with((AsyncRT*)weaver, &task)

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
