#define Asynx_CREATE(name, ...) \
	LITERAL(name##_Task, \
		.entry__ = &name##_entry, \
		.return__ = AsynxReturn_None, \
		__VA_ARGS__ \
	)

#define Asynx_CREATE_RESOLVE(name, future, ...) \
	LITERAL(name##_Task, \
		.entry__ = &name##_entry, \
		.return__ = AsynxReturn_Resolve, \
		.return_future__ = Asynx_UPCASTFUTURE(future), \
		__VA_ARGS__ \
	)

#define Asynx_SET_RESOLVE(task, future) { \
	(task)->return__ = AsynxReturn_Resolve; \
	(task)->return_future__ = Asynx_UPCASTFUTURE(future); \
}
