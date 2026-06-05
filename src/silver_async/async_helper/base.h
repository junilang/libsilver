#define async_TOK__(pfx, tok) pfx##tok
#define async_TOK_(pfx, tok) async_TOK__(pfx, tok)
#define async_TOK(tok) async_TOK_(async_PFX, tok)

#define async_LABELS_(PFX, ...) ENUM(PFX, async_Label_TYPE, 1, __VA_ARGS__)
#define async_LABELS(...) async_LABELS_(async_TOK(_Label_), __VA_ARGS__);

typedef struct async_TASK_STRUCT_BODY() async_TASK_Generic;

constexpr usize async_TASK_FORK_SYNC_OFFSET =
	offsetof(async_TASK_Generic, fork_sync__);

#define async_STATE(...) \
	typedef STRUCTDECL(async_TOK(_Task)); \
	struct async_TOK(_Task) async_TASK_STRUCT_BODY(__VA_ARGS__); \
	static_assert( \
		offsetof(async_TOK(_Task), fork_sync__) == async_TASK_FORK_SYNC_OFFSET, \
		"fork_sync__ member offset does not match generic struct" \
	);

#define async_LOCAL_THIS \
	[[maybe_unused]] auto const this = async_data__;

#define async_BEGIN \
	AsyncIntent async_TOK(_entry)( \
		AsyncRT *const async_rt__, Ptr const async_vdata__, AsyncTaskState const async_state__, \
		AsyncResult *const async_result__ \
	) { \
		[[maybe_unused]] \
		auto async_fork_id__ = async_GET_FORK_ID(async_state__); \
		auto const async_label__ = async_GET_LABEL(async_state__); \
		async_TOK(_Task) *const async_data__ = async_vdata__; \
		async_LOCALS \
		switch (async_label__) { \
			case 0:

#define async_RETURN_JOIN_SYNC { \
	auto const sync = (_Atomic async_ForkSync_TYPE*) \
		((ubyte*)async_data__->return_join_data__ + async_TASK_FORK_SYNC_OFFSET); \
	if (atomic_fetch_sub_explicit(sync, 1, memory_order_acq_rel) != 1) \
		return AsyncIntent_YIELD; \
}

#define async_END \
		} \
		[[maybe_unused]] \
		async_return__:; \
		switch (async_data__->return__) { \
			case async_Return_NONE: \
				return AsyncIntent_YIELD; \
			case async_Return_RESUME: \
				async_result__->task = async_data__->return_task__; \
				return AsyncIntent_RESUME; \
			case async_Return_SUSPEND: \
				async_result__->task = async_data__->return_task__; \
				return AsyncIntent_SUSPEND; \
			case async_Return_RESOLVE: \
				return AsyncFuture_resolve(async_data__->return_future__, async_result__); \
			case async_Return_JOIN_RESUME: \
				async_RETURN_JOIN_SYNC \
				async_result__->task = AsyncTask_upcast(async_data__->return_join_data__, \
					async_MAKE_TASK_STATE(0, async_data__->return_join_label__) \
				); \
				return AsyncIntent_RESUME; \
			case async_Return_JOIN_SUSPEND: \
				async_RETURN_JOIN_SYNC \
				async_result__->task = AsyncTask_upcast(async_data__->return_join_data__, \
					async_MAKE_TASK_STATE(0, async_data__->return_join_label__) \
				); \
				return AsyncIntent_SUSPEND; \
		} \
		UNREACHABLE; \
	}

#define async_LABEL(label) \
	[[fallthrough]]; case async_TOK(_Label_##label): \
	[[maybe_unused]] async_label_##label##__:

#define async_SELF(label) \
	AsyncTask_upcast(async_data__, async_MAKE_TASK_STATE(async_fork_id__, async_TOK(_Label_##label)))
