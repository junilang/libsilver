typedef struct {
	void (*submit)(Ptr this, const AsyncTask *tasks, usize tasks_size);
	Ptr (*alloc_task)(Ptr this, usize *io_size, uint n);
	Ptr (*alloc_task_array)(Ptr this, usize *io_sizes, uint n);
	void (*delete_task)(Ptr this, Ptr buf);
} AsyncRT;

void AsyncRT_submit(AsyncRT *this, const AsyncTask *tasks, usize tasks_size) {
	this->submit(this, tasks, tasks_size);
}

Ptr AsyncRT_alloc_task(AsyncRT *this, usize *io_size, uint n) {
	return this->alloc_task(this, io_size, n);
}

Ptr AsyncRT_alloc_tasks(AsyncRT *this, usize *io_sizes, uint n) {
	return this->alloc_task_array(this, io_sizes, n);
}

void AsyncRT_delete_task(AsyncRT *this, Ptr buf) {
	this->delete_task(this, buf);
}
