#define async_WAIT_IMMEDIATE_TO(label, future) { \
	if (AsyncFuture_set((future), async_SELF(label))) { \
		return AsyncIntent_YIELD; \
	} \
	async_GOTO(label); \
}

#define async_WAIT_IMMEDIATE(label, future) \
	async_WAIT_IMMEDIATE_TO(label, future) \
	async_LABEL(label)

#define async_WAIT_RESUME_TO(label, future) { \
	auto const async_self__ = async_SELF(label); \
	if (AsyncFuture_set((future), async_self__)) { \
		return AsyncIntent_YIELD; \
	} \
	async_io__->out_task = async_self; \
	return AsyncIntent_RESUME; \
}

#define async_WAIT_RESUME(label, future) \
	async_WAIT_RESUME_TO(label, future) \
	async_LABEL(label)

#define async_WAIT_SUSPEND_TO(label, future) { \
	auto const async_self__ = async_SELF(label); \
	if (AsyncFuture_set((future), async_self__)) { \
		return AsyncIntent_YIELD; \
	} \
	async_io__->out_task = async_self; \
	return AsyncIntent_SUSPEND; \
}

#define async_WAIT_SUSPEND(label, future) \
	async_WAIT_SUSPEND_TO(label, future) \
	async_LABEL(label)
