#define async ZZmergesort
	async_state(
		Allocator alc;
		ZZmergesort_Task *children;
		int *src, *dst;
		uint size;
	)
	async_labels(fork, join)

	async_begin {
		if (this->size > 4)
			this->children = Allocator_new(this->alc, sizeof(ZZmergesort_Task) * 2);
	}

	async_fork(fork, 2) {
		int *src = this->src;
		int *dst = this->dst;
		uint size = this->size / 2;

		if (async_fork_id__ == 2) {
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
				FALLTHROUGH;
			case 1:
			case 0:
				async_join_to(join);
		}

		auto child = &this->children[async_fork_id__ - 1];

		*child = async_create(ZZmergesort,
			.alc = this->alc,
			.src = src, .dst = dst, .size = size
		);

		async_call_join_to(join, child);
	}

	async_label(join) {
		if (this->children)
			Allocator_delete(this->alc, this->children);

		uint ls = this->size / 2;
		int *left = this->dst;
		int *left_end = left + ls;
		int *right = left_end;
		int *right_end = left + this->size;

		int *dst = this->src;

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

int ZZentry(SilverTestContext *ctx) {
	Weaver *weaver = Weaver_create(Malloc, 8, 256);
	Weaver_boot(weaver);

	constexpr auto array_size = 10240000;
	int *array = Allocator_new(Malloc, sizeof(int) * array_size * 2);
	int *result = array + array_size;

	srand((uint)time(nullptr));

	for (uint i = 0; i < array_size; i++) {
		array[i] = rand() % array_size;
	}

	AsyncFuture_Mtx future;
	AsyncFuture_init(&future);

	auto task = async_create_resolve(ZZmergesort, &future,
		.alc = Malloc,
		.src = array,
		.dst = result,
		.size = array_size
	);

	struct timespec bench_begin;
	clock_gettime(CLOCK_MONOTONIC, &bench_begin);

	async_spawn_with((AsyncRT*)weaver, &task);

	AsyncFuture_wait(&future);

	struct timespec bench_end;
	clock_gettime(CLOCK_MONOTONIC, &bench_end);

	auto ts = bench_end.tv_sec - bench_begin.tv_sec;
	auto ns = bench_end.tv_nsec - bench_begin.tv_nsec;

	double ms = (double)(ts * 1000) + ((double)ns / 1'000'000.);

	FPRINT(stdout,
		"sorting ",array_size," elements took: ",ms,"ms\n"
	);


	#if 0

		FPRINT(stdout, "result = {")
		for (uint i = 0; i < (array_size - 1); i++) {
			FPRINT(stdout, result[i],", ");
		}
		FPRINT(stdout, result[array_size -1],"}\n");

	#endif

	Allocator_delete(Malloc, array);

	Weaver_join(weaver);
	Weaver_destroy(weaver);

	return SilverTest_OK;
}
