#define async_YIELD \
	return AsyncIntent_YIELD

#define async_RETURN \
	goto async_return__

#define async_GOTO(label) \
	goto async_label_##label##__

#define async_RESUME_TO(label) { \
	async_result__->task = async_SELF(label); \
	return AsyncIntent_RESUME; \
}

#define async_RESUME(label) \
	async_RESUME_TO(label) \
	async_LABEL(label)

#define async_SUSPEND_TO(label) { \
	async_result__->task = async_SELF(label); \
	return AsyncIntent_SUSPEND; \
}

#define async_SUSPEND(label) \
	async_SUSPEND_TO(label) \
	async_LABEL(label)
