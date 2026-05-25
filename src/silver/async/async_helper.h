#define ASYNCFNPFX__(PFX, TOK) PFX##TOK
#define ASYNCFNPFX_(PFX, TOK) ASYNCFNPFX__(PFX, TOK)
#define ASYNCFNPFX(TOK) ASYNCFNPFX_(async, TOK)

#define async_state(...) \
	typedef STRUCTDECL(ASYNCFNPFX(_Task)); \
	typedef struct { __VA_ARGS__ } ASYNCFNPFX(_State); \
	struct ASYNCFNPFX(_Task) { \
		AsyncTaskData head; \
		alignas(max_align_t) ASYNCFNPFX(_State) state; \
	};

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

#define async_new(N, alc) ((N##_Task*)Allocator_new(alc, sizeof(N##_Task)))

#define async_spawn(task, future_) { \
	(task)->head.future = (future_); \
	AsyncRT_submit(g_async_rt, \
		(AsyncTask[]){AsyncTask_setmeta(&(task)->head, AsyncTaskMetadata_create(0,0))}, 1 \
	); \
}

#define async_spawn_many(future_, ...) { \
	Ptr tasks[] = {__VA_ARGS__}; \
	auto const size = sizeof(tasks) / sizeof(Ptr); \
	for (usize i = 0; i < size; i++) { \
		((AsyncTaskData*)tasks[i])->future = (future_); \
		tasks[i] = (Ptr)(AsyncTask_setmeta(tasks[i], AsyncTaskMetadata_create(0,0)).value); \
	} \
	AsyncRT_submit(g_async_rt, (Ptr)tasks, size); \
}

#define async_call(task, label) { \
	(task)->head.resume = AsyncTask_setmeta(AsyncTask_data(g_async_task), \
		AsyncTaskMetadata_create(AsyncTask_flags(g_async_task), ASYNCFNPFX(_Label_##label)) \
	); \
	return (AsyncResult){ \
		.next = AsyncTask_setmeta(&(task)->head, \
			AsyncTaskMetadata_create(FLAG(AsyncTaskFlag, RESUMING), 0) \
		) \
	}; \
}

#define async_yield(label) { \
	return (AsyncResult) { \
		.next = AsyncTask_setmeta(AsyncTask_data(g_async_task), \
			AsyncTaskMetadata_create( \
				AsyncTask_metadata(g_async_task) | FLAG(AsyncTaskFlag, YIELD), \
				ASYNCFNPFX(_Label_##label) \
			) \
		) \
	}; \
}

#define async_resume(future, label) \
	if ( \
		_Generic((future), \
			AsyncFuture_Resume* : AsyncFuture_Resume_set, \
			AsyncFuture_CountResume* : AsyncFuture_CountResume_set \
		)((future), \
			AsyncTask_setmeta(AsyncTask_data(g_async_task), \
				AsyncTaskMetadata_create( \
					AsyncTask_metadata(g_async_task), ASYNCFNPFX(_Label_##label) \
				) \
			) \
		) \
	) return AsyncResult_YIELD; \
	async_label(label)


#define async_wait(future) _Generic((future), \
	AsyncFuture_Wake* : AsyncFuture_Wake_wait, \
	AsyncFuture_CountWake* : AsyncFuture_CountWake_wait \
)(future)

#define async_suspend return AsyncResult_YIELD
#define async_return return AsyncResult_FINISH
