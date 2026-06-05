#define async_CALL_RESUME_TO(label, task_) { \
	(task_)->return_task__ = async_SELF(label); \
	(task_)->return__ = async_Return_RESUME; \
	async_result__->task = AsyncTask_upcast((task_), async_NULL_TASK_STATE); \
	return AsyncIntent_CALL; \
}

#define async_CALL_RESUME(label, task) \
	async_CALL_RESUME_TO(label, task) \
	async_LABEL(label)

#define async_CALL_SUSPEND_TO(label, task_) { \
	(task_)->return_task__ = async_SELF(label); \
	(task_)->return__ = async_Return_SUSPEND; \
	async_result__->task = AsyncTask_upcast((task_), async_NULL_TASK_STATE); \
	return AsyncIntent_CALL; \
}

#define async_CALL_SUSPEND(label, task) \
	async_CALL_SUSPEND_TO(label, task) \
	async_LABEL(label)

#define async_CALL_JOIN_RESUME_TO(label, task_) { \
	(task_)->return_join_data__ = async_data__; \
	(task_)->return_join_label__ = async_TOK(_Label_##label); \
	(task_)->return__ = async_Return_JOIN_RESUME; \
	async_result__->task = AsyncTask_upcast((task_), async_NULL_TASK_STATE); \
	return AsyncIntent_CALL; \
}

#define async_CALL_JOIN_RESUME(label, task) \
	async_CALL_JOIN_RESUME_TO(label, task) \
	async_LABEL(label)

#define async_CALL_JOIN_SUSPEND_TO(label, task_) { \
	(task_)->return_join_data__ = async_data__; \
	(task_)->return_join_label__ = async_TOK(_Label_##label); \
	(task_)->return__ = async_Return_JOIN_SUSPEND; \
	async_result__->task = AsyncTask_upcast((task_), async_NULL_TASK_STATE); \
	return AsyncIntent_CALL; \
}

#define async_CALL_JOIN_SUSPEND(label, task) \
	async_CALL_JOIN_SUSPEND_TO(label, task) \
	async_LABEL(label)
