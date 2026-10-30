typedef struct {
	AsyncRes (*submit)(Ptr this, const AsyncTask *tasks, usize tasks_size);
	AlcPtr (*alc)(Ptr this, AlcReq *req, Ptr arg, Ptr mem);
} AsyncRT;

AsyncRes AsyncRT_submit(AsyncRT *this, const AsyncTask *tasks, usize tasks_size) {
	return this->submit(this, tasks, tasks_size);
}

AlcPtr AsyncRT_alc(AsyncRT *this, AlcReq *req, Ptr arg, Ptr mem) {
	return this->alc(this, req, arg, mem);
}
