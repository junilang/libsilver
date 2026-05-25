#define async ZZadd
	async_state(union { int x; int result; }; int y, z;)
	async_begin
	{
		FPRINT(stdout, "adder: ",this->x," ",this->y,"\n");
		this->result = this->x + this->y;
		async_return;
	}
	async_end
#undef async

#define async ZZsub
	async_state(ZZadd_Task add_task;)
	async_labels(after_add)
	async_begin
	{
		FPRINT_ATOMIC(stderr, "sub: hello\n");
		async_init(ZZadd, &this->add_task, .x = 10, .y = 5);
		async_call(&this->add_task, after_add);
		async_label(after_add);

		FPRINT_ATOMIC(stdout, "adder result: ",this->add_task.state.result,"\n");
	}
	async_end
#undef async

#define async ZZsleep
	async_state(uint time;)
	async_begin
	{
		FPRINT_ATOMIC(stdout, "sleep start\n");
		sleep(this->time);
		FPRINT_ATOMIC(stdout, "sleep done\n");
		async_return;
	}
	async_end
#undef async

int ZZentry(SilverTestContext *ctx) {
	Weaver *weaver = Weaver_create(Malloc, 4, 8);
	Weaver_boot(weaver);

	AsyncRT g_async_rt = Weaver_upcast(weaver);

	AsyncFuture_CountWake wake_;
	AsyncFuture wake = AsyncFuture_CountWake_init(&wake_, 2);

	auto task = async_new(ZZsub, Malloc);
	async_init(ZZsub, task);

	auto task2 = async_new(ZZsleep, Malloc);
	async_init(ZZsleep, task2, .time = 1)

	async_spawn(task2, wake);
	async_spawn(task, wake);

	AsyncFuture_CountWake_wait(&wake_);

	Weaver_join(weaver);
	Weaver_destroy(weaver);

	Allocator_delete(Malloc, task);
	Allocator_delete(Malloc, task2);

	return SilverTest_OK;
}
