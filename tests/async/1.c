#define async ZZasync_add
	async_state(int x; int y;)
	async_labels(fork, join)

	async_begin
		async_fork(fork, 4)

			FPRINT_ATOMIC(stdout, "async_add: fork ",async_fork_id__,"\n");

		async_join(join)

		FPRINT_ATOMIC(stdout, "async_add: ",this->x," + ",this->y,"\n");
		this->x += this->y;

	async_end
#undef async

int ZZentry(SilverTestContext *ctx) {
	Weaver *weaver = Weaver_create(Malloc, 4, 2);
	Weaver_boot(weaver);

	AsyncFuture_Mtx mtx;
	auto task = async_create_resolve(ZZasync_add, AsyncFuture_init(&mtx), .x = 5, .y = 10);

	async_spawn_with((AsyncRunner*)weaver, &task);

	AsyncFuture_wait(&mtx);
	FPRINT(stdout, "result: ", task.x, "\n");

	Weaver_join(weaver);
	Weaver_destroy(weaver);

	return SilverTest_OK;
}
