typedef struct {
	Allocator queue_alc;
	AsyncRT rt;

	_Atomic RioQinfo qinfo;
	_Atomic RioQinfo mqinfo;

	RioQueue *queue[2];

	u16 workers_size;
	RioWorker workers[];
} Rio;

usize ZZRio_allocsize(u16 workers_size) {
	return offsetof(Rio, workers) + (sizeof(RioWorker) * workers_size);
}

void Rio_init(Rio *this,
	AsyncRT rt, u16 workers, u64 queue_capacity, Allocator queue_alc
) {
	this->queue_alc = queue_alc;
	this->work
}
