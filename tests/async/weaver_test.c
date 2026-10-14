typedef union {
	_Alignas(cpu_hdi_size) char align__;
	struct {
		AsyncFn entry;
	};
} ZZtest_Task;

AsyncIntent ZZtest_entry(Ptr data, AsyncRT *rt, AsyncTaskArgs args, AsyncResult *result) {
	PRINTBP(128, Stdout, "Hello weaver!\n");
	return AsyncIntent_Yield;
}

constexpr u32 ZZthreads = 2;

_Alignas(Weaver) ubyte ZZweaver_[Weaver_ALLOCSIZE(ZZthreads)];

int ZZentry(SilverTestContext *ctx) {
	StaticAlc_LOCAL(alc, 16384);

	auto weaver = (Weaver*)ZZweaver_;
	AlcRes_UNWRAP(
		Weaver_init(weaver, alc, ZZthreads, 8)
	);

	Weaver_boot(weaver);

	_Alignas(cpu_hdi_size) ZZtest_Task task_data = {
		.entry = &ZZtest_entry
	};

	AsyncTask task = AsyncTask_upcast(&task_data, 0);
	AlcRes_UNWRAP(
		Weaver_submit(weaver, &task, 1)
	);

	Async_sleep(Async_millis * 500);

	//PRINTBP(128, Stdout, "nanosleep_res: ",res,"\n");

	Weaver_join(weaver);

	return 0;
}
