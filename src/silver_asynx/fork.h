typedef enum : u8 {
	AsynxForkErr_None,
	AsynxForkErr_Recursive,
	AsynxForkErr_Overflow,
	AsynxForkErr_Zero,
	AsynxForkErr_One
} AsynxForkErr;

AsynxForkErr Asynx_forkvalidate(AsynxForkId id, usize n) {
	#if Asynx_SAFE
		if (id != 0)
			return AsynxForkErr_Recursive;

		if (n > AsynxForkId_max)
			return AsynxForkErr_Overflow;

		if (n == 0)
			return AsynxForkErr_Zero;

		if (n == 1)
			return AsynxForkErr_One;
	#endif

	return AsynxForkErr_None;
}

constexpr usize Asynx_fsasize = Asynx_FSASIZE;

AlcRes Asynx_fork(AsyncRT *rt, Ptr data, AsynxForkId count, AsyncTaskState state) {
	AsyncTask tasks[Asynx_fsasize];
	do {
		AsynxForkId const n = count < Asynx_fsasize ? count : Asynx_fsasize;
		for (AsynxForkId i = 0; i < n; i++) {
			tasks[i] = AsyncTask_upcast(data, state);
			state += AsynxForkId_one;
		}
		count -= n;

		auto res = AsyncRT_submit(rt, tasks, n);
		if (res) return res;
	} while (count);

	return AlcRes_Ok;
}

#define Asynx_SETFORK(data, n) { \
	(data)->fork_count__ = n; \
	(data)->fork_sync__ = n; \
}

#define Asynx_FORK_COMMON(n_) \
	asynx_error__.fork = Asynx_validatefork(asynx_fork_id__, (n_)); \
	if (asynx_error__.fork) goto asynx_catch_fork__;\
	auto const n = (AsynxForkId)(n_); \
	Asynx_SETFORK(asynx_data__, n); \

#define Asynx_FORK_IMMEDIATE_TO(lbl, n_) { \
	Asynx_FORK_COMMON(n_) \
	auto const state = Asynx_makestate(2, Asynx_TOK(_label_##lbl)); \
	asynx_error__ = Asynx_fork(asynx_rt__, asynx_data__, n - 1, state); \
	if (res)
	asynx_fork_id__ = 1; \
	goto asynx_label_##lbl##__; \
}

#define Asynx_FORK_RESUME_TO(lbl, n_) { \
	Asynx_FORK_COMMON(n_) \
	auto state = Asynx_makestate(1, Asynx_TOK(_label##lbl)); \
	auto res = Asynx_fork(asynx_rt__, asynx_data__, n - 1, state + AsynxForkId_one); \
	if (res)
	asynx_result__->task = AsyncTask_upcast(asynx_data__, state); \
	return AsyncIntent_Resume;
}
