#define async_SPAWN_WITH(rt, ...) { \
	Ptr tp[] = {__VA_ARGS__}; \
	constexpr usize n = sizeof(tp) / sizeof(Ptr); \
	AsyncTask tasks[n]; \
	for (usize i = 0; i < n; i++) { \
		tasks[i] = AsyncTask_upcast(tp[i], async_NULL_TASK_STATE); \
	} \
	AsyncRT_submit(rt, tasks, n); \
}

#define async_SPAWN(...) async_SPAWN_WITH(async_rt__, __VA_ARGS__)
