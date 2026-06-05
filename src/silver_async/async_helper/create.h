#define async_ALLOCSIZE(name) sizeof(name##_Task)

#define async_CREATE(name, ...) \
	LITERAL(name##_Task, \
		.entry__ = &name##_entry, \
		.return__ = async_Return_NONE, \
		__VA_ARGS__ \
	)

#define async_CREATE_RESOLVE(name, future, ...) \
	LITERAL(name##_Task, \
		.entry__ = &name##_entry, \
		.return__ = async_Return_RESOLVE, \
		.return_future__ = AsyncFuture_upcast(future), \
		__VA_ARGS__ \
	)

#define async_SET_RESOLVE(task, future) { \
	(task)->return__ = async_Return_RESOLVE; \
	(task)->return_future__ = AsyncFuture_upcast(future); \
}
