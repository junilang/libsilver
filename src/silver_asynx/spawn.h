#define Asynx_SPAWN_WITH(rt, ...) { \
	Ptr tp[] = {__VA_ARGS__}; \
	constexpr usize n = COUNTOF(tp); \
	AsyncTask tasks[n]; \
	for (usize i = 0; i < n; i++) { \
		tasks[i] = AsyncTask_upcast(tp[i], Asynx_nullstate); \
	} \
	auto const res = AsyncRT_submit(rt, asynx_context__, tasks, n); \
	if (res) PANIC("handle error"); \
}

#define Asynx_SPAWN(...) Asynx_SPAWN_WITH(asynx_rt__, __VA_ARGS__)
