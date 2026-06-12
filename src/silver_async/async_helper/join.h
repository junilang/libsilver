#define async_JOIN_SYNC { \
	if (atomic_fetch_sub_explicit(&async_data__->fork_sync__, 1, memory_order_acq_rel) != 1) \
		return AsyncIntent_YIELD; \
}

#define async_JOIN_IMMEDIATE_TO(label) { \
	async_JOIN_SYNC \
	async_fork_id__ = 0; \
	goto async_label_##label##__; \
}

#define async_JOIN_IMMEDIATE(label) \
	async_JOIN_IMMEDIATE_TO(label) \
	async_LABEL(label)

#define async_JOIN_RESUME_TO(label) { \
	async_JOIN_SYNC \
	async_io__->out_task = AsyncTask_upcast( \
		async_data__, async_MAKE_TASK_STATE(0, async_TOK(_Label_##label)) \
	); \
	return AsyncIntent_RESUME; \
}

#define async_JOIN_RESUME(label) \
	async_JOIN_RESUME_TO(label) \
	async_LABEL(label)

#define async_JOIN_SUSPEND_TO(label) { \
	async_JOIN_SYNC \
	async_io__->out_task = AsyncTask_upcast( \
		async_data__, async_MAKE_TASK_STATE(0, async_TOK(_Label_##label)) \
	); \
	return AsyncIntent_SUSPEND; \
}

#define async_JOIN_SUSPEND(label) \
	async_JOIN_SUSPEND_TO(label) \
	async_LABEL(label)
