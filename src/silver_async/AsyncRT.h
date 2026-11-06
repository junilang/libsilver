typedef struct {
	AsyncRes (*submit)(Ptr this, usize context, const AsyncTask *tasks, usize tasks_size);
	#if Alc_PTRTAG
		Alc (*alc)(Ptr this, usize context);
	#else
		void (*alc)(Ptr this, usize context, Alc *out_alc);
	#endif
} AsyncRT;

AsyncRes AsyncRT_submit(AsyncRT *this, usize context, const AsyncTask *tasks, usize tasks_size) {
	return this->submit(this, context, tasks, tasks_size);
}

Alc AsyncRT_alc(AsyncRT *this, usize context) {
	#if Alc_PTRTAG
		return this->alc(this, context);
	#else
		Alc alc;
		this->alc(this, context, &alc);
		return alc;
	#endif
}
