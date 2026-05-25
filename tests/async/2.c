#define async ZZmergesort
	async_state(
		Allocator alc;
		AsyncFuture_CountResume future;
		ZZmergesort_Task *children;
		int *src, *dst;
		uint size;
	)
	async_labels(merge)
	async_begin {
		switch (this->size) {
			case 0:
			case 1:
				this->dst[0] = this->src[0];
				async_return;

			case 2: {
				if (this->src[0] > this->src[1]) {
					this->dst[0] = this->src[1];
					this->dst[1] = this->src[0];
				} else {
					this->dst[0] = this->src[0];
					this->dst[1] = this->src[1];
				}
				async_return;
			}
		}

		// split into two arrays
		uint l = this->size / 2;

		this->children = Allocator_new(this->alc, sizeof(ZZmergesort_Task) * 2);

		async_init(ZZmergesort, &this->children[0],
			.alc = this->alc,
			.src = this->src,
			.dst = this->dst,
			.size = l
		);

		async_init(ZZmergesort, &this->children[1],
			.alc = this->alc,
			.src = this->src + l,
			.dst = this->dst + l,
			.size = this->size - l
		);

		auto future = AsyncFuture_CountResume_init(&this->future, 2);

		async_spawn_many(future, &this->children[0], &this->children[1]);
	}

	async_resume(&this->future, merge) {
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

		Allocator_delete(this->alc, this->children);
		async_return;
	}

	async_end
#undef async

int ZZentry(SilverTestContext *ctx) {
	Weaver *weaver = Weaver_create(Malloc, 4, 2);
	Weaver_boot(weaver);

	AsyncRT g_async_rt = Weaver_upcast(weaver);

	constexpr auto array_size = 1000000;
	int *array = Allocator_new(Malloc, sizeof(int) * array_size * 2);
	int *result = array + array_size;

	for (uint i = 0; i < array_size; i++) {
		array[i] = rand() % 1000;
	}

	ZZmergesort_Task task;
	async_init(ZZmergesort, &task,
		.alc = Malloc,
		.src = array,
		.dst = result,
		.size = array_size
	);

	AsyncFuture_Wake wake;
	AsyncFuture_Wake_init(&wake);

	async_spawn(&task, AsyncFuture_Wake_upcast(&wake));

	async_wait(&wake);

	/*
	FPRINT(stdout, "result = {")
	for (uint i = 0; i < (array_size - 1); i++) {
		FPRINT(stdout, result[i],", ");
	}
	FPRINT(stdout, result[array_size -1],"}\n");
	*/

	FPRINT(stdout, "done\n");

	Allocator_delete(Malloc, array);

	Weaver_join(weaver);
	Weaver_destroy(weaver);

	return SilverTest_OK;
}
