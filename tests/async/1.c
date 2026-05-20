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
	async_state(ZZadd_Task add_task; Spinlock sync_lock;)
	async_labels(after_add)
	async_begin
	{
		FPRINT(stderr, "sub: hello\n");
		async_init(ZZadd, &this->add_task, .x = 10, .y = 5);
		async_yield(&this->add_task, after_add);

		async_label(after_add);

		FPRINT(stdout, "adder result: ",this->add_task.state.result,"\n");

		Spinlock_release(&this->sync_lock);
	}
	async_end
#undef async

int ZZentry(SilverTestContext *ctx) {
	Weaver *weaver = Weaver_create(Malloc, 1, 8);
	Weaver_boot(weaver);

	AsyncRT g_async_rt = Weaver_upcast(weaver);

	{
		async_new(ZZsub, Malloc);
		Spinlock_init(&task->state.sync_lock);
		Spinlock_acquire(&task->state.sync_lock);

		async_disown(task);

		Spinlock_acquire(&task->state.sync_lock);

		Allocator_delete(Malloc, task);
	}

	Weaver_join(weaver);
	Weaver_destroy(weaver);

	return SilverTest_OK;
}
