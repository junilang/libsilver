typedef struct {
	AsyncRes (*submit)(Ptr this, usize context, const AsyncTask *tasks, usize tasks_size);
	AsyncRes (*alc)(Ptr this, usize context, Alc *out_alc);
} AsyncRT;

AsyncRes AsyncRT_submit(AsyncRT *this, usize context, const AsyncTask *tasks, usize tasks_size) {
	return this->submit(this, context, tasks, tasks_size);
}

AsyncRes AsyncRT_alc(AsyncRT *this, usize context, Alc *out_alc) {
	return this->alc(this, context, out_alc);
}
