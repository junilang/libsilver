async_ForkId_TYPE async_FORK_validate(async_ForkId_TYPE fid, usize n) {
	#if BUILD_SAFE
		if (fid != 0)
			PANIC("cannot recursively fork an async task");

		if (n > async_ForkId_MAX)
			PANIC("requested too many forks");

		if (n == 0)
			PANIC("requested 0 forks");
	#endif

	return (async_ForkId_TYPE)n;
}

void async_FORK_submit_all(
	AsyncRT *rt, Ptr data, async_ForkId_TYPE n, async_Label_TYPE label
) {
	AsyncTask *tasks = alloca(sizeof(AsyncTask) * n);
	for (async_ForkId_TYPE i = 0; i < n; i++) {
		tasks[i] = AsyncTask_upcast(data, async_MAKE_TASK_STATE(i + 1, label));
	}
	AsyncRT_submit(rt, tasks, n);
}

void async_FORK_submit_rest(
	AsyncRT *rt, Ptr data, async_ForkId_TYPE n, async_Label_TYPE label
) {
	AsyncTask *tasks = alloca(sizeof(AsyncTask) * n);
	for (async_ForkId_TYPE i = 0; i < n; i++) {
		tasks[i] = AsyncTask_upcast(data, async_MAKE_TASK_STATE(i + 2, label));
	}
	AsyncRT_submit(rt, tasks, n);
}

#define async_FORK_SET_STATE(n) { \
	async_data__->fork_count__ = n; \
	async_data__->fork_sync__ = n; \
}

#define async_FORK_IMMEDIATE_TO(label, n_) { \
	auto const n = async_FORK_validate(async_fork_id__, (n_)); \
	async_FORK_SET_STATE(n) \
	if (n != 1) \
		async_FORK_submit_rest(async_rt__, async_data__, n - 1, async_TOK(_Label_##label)); \
	async_fork_id__ = 1; \
	goto async_label_##label##__; \
}

#define async_FORK_IMMEDIATE(label, n) \
	async_FORK_IMMEDIATE_TO(label, n) \
	async_LABEL(label)

#define async_FORK_RESUME_TO(label, n_) { \
	auto const n = async_FORK_validate(async_fork_id__, (n_)); \
	async_FORK_SET_STATE(n) \
	if (n != 1) \
		async_FORK_submit_rest(async_rt__, async_data__, n - 1, async_TOK(_Label_##label)); \
	async_result__->task = AsyncTask_upcast( \
		async_data__, async_MAKE_TASK_STATE(1, async_TOK(_Label_##label)) \
	); \
	return AsyncIntent_RESUME; \
}

#define async_FORK_RESUME(lable, n_) \
	async_FORK_RESUME_TO(label, n) \
	async_LABEL(label)

#define async_FORK_SUSPEND_TO(label, n_) { \
	auto const n = async_FORK_validate(async_fork_id__, (n_)); \
	async_FORK_SET_STATE(n) \
	if (n != 1) { \
		async_FORK_submit_all(async_rt__, async_data__, n, async_TOK(_Label_##label)); \
		return AsyncIntent_YIELD; \
	} else { \
		async_result__->task = AsyncTask_upcast( \
			async_data__, async_MAKE_TASK_STATE(1, async_TOK(_Label_##label)) \
		); \
		return AsyncIntent_SUSPEND; \
	} \
}

#define async_FORK_SUSPEND(label, n) \
	async_FORK_SUSPEND_TO(label, n) \
	async_LABEL(label)
