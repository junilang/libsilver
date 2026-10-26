#define async ZZtest
	async_state(
		_Atomic u32_lf *target;
	)
	async_labels(fork, join)

	async_begin


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
	StaticAlc_LOCAL(alc, 16384);

	auto weaver = (Weaver*)ZZweaver_;
	AlcRes_UNWRAP(
		Weaver_init(weaver, alc, ZZthreads, 8)
	);

	Weaver_boot(weaver);

	AsyncFuture future;
	AsyncFuture_init(&future, 1);

	_Atomic u32_lf counter = 0;

	ZZtest_Task task_data =
		async_create_resolve(ZZtest, &future,
			.target = &counter
		)
	;

	AsyncTask task = AsyncTask_upcast(&task_data, 0);
	AlcRes_UNWRAP(
		Weaver_submit(weaver, &task, 1)
	);

	//Async_sleep(Async_millis * 500);


	AsyncFuture_wait(&future, 0);
	//PRINTBP(128, Stdout, "nanosleep_res: ",res,"\n");
	//
	PRINTBP(128, Stdout, "result: ",atom_get(&counter, atom_acq),"\n");

	Weaver_join(weaver);

	return 0;
}
