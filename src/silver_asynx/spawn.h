#define Asynx_SPAWN_WITH(rt, ...) { \
	Ptr tp[] = {__VA_ARGS__}; \
	constexpr usize n = COUNTOF(tp); \
	AsyncTask tasks[n]; \
	for (usize i = 0; i < n; i++) { \
		tasks[i] = AsyncTask_upcast(tp[i], Asynx_nullstate); \
	} \
	auto res = AsyncRT_submit(rt, tasks, n); \
	if (res) TODO("handle error"); \
}

#define Asynx_SPAWN(...) Asynx_SPAWN_WITH(asynx_rt__, __VA_ARGS__)
