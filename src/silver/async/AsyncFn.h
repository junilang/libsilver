#define ASYNCFNPFX__(PFX, TOK) PFX##TOK
#define ASYNCFNPFX_(PFX, TOK) ASYNCFNPFX__(PFX, TOK)
#define ASYNCFNPFX(TOK) ASYNCFNPFX_(async, TOK)

#define async_state(...) \
	typedef struct { __VA_ARGS__ } ASYNCFNPFX(_State); \
	typedef struct { \
		AsyncTaskData head; \
		alignas(max_align_t) ASYNCFNPFX(_State) state; \
	} ASYNCFNPFX(_Task);

#define ASYNC_LABELS_(PFX, ...) ENUM(PFX, AsyncTaskState, 1, __VA_ARGS__);
#define async_labels(...) ASYNC_LABELS_(ASYNCFNPFX(_Label_), __VA_ARGS__)

#define async_begin \
	AsyncResult ASYNCFNPFX(_entry)(AsyncRT g_async_rt, AsyncTask g_async_task) { \
		auto this = &((ASYNCFNPFX(_Task)*)AsyncTask_data(g_async_task))->state; \
		switch (AsyncTask_state(g_async_task)) {\
			default: PANIC("async invalid state"); \
			case 0:

#define async_end \
		} \
		return AsyncResult_FINISH; \
	}

#define async_label(N) FALLTHROUGH; case ASYNCFNPFX(_Label_##N):

#define async_init(N, task, ...) { \
	(task)->head.entry = &N##_entry; \
	(task)->state = LITERAL(N##_State, __VA_ARGS__); \
}

#define async_new(N, alc, ...) \
	N##_Task *task = Allocator_new(alc, sizeof(N##_Task)); \
	async_init(N, task, __VA_ARGS__)

#define async_disown(task) { \
	(task)->head.future = AsyncFuture_NULL; \
	AsyncRT_submit(g_async_rt, \
		(AsyncTask[]){AsyncTask_setmeta(&(task)->head, AsyncTaskMetadata_create(0,0))}, 1 \
	); \
}

#define async_yield(task, label) { \
	(task)->head.resume = AsyncTask_setmeta(AsyncTask_data(g_async_task), \
		AsyncTaskMetadata_create(AsyncTask_flags(g_async_task), ASYNCFNPFX(_Label_##label)) \
	); \
	return (AsyncResult){ \
		.next=AsyncTask_setmeta(&(task)->head, \
			AsyncTaskMetadata_create(FLAG(AsyncTaskFlag, RESUMING), 0) \
		) \
	}; \
}

#define async_suspend return AsyncResult_SUSPEND
#define async_return return AsyncResult_FINISH
#define async_panic return AsyncResult_PANIC
