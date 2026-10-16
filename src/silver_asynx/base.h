#define Asynx_TOK__(pfx, tok) pfx##tok
#define Asynx_TOK_(pfx, tok) Asynx_TOK__(pfx, tok)
#define Asynx_TOK(tok) Asynx_TOK_(Asynx_PFX, tok)

#define Asynx_LABELS_(pfx, ...) ENUM(pfx, AsynxLabel, 1, __VA_ARGS__)
#define Asynx_LABELS(...) Asynx_LABELS_(Asynx_TOK(_label_), __VA_ARGS__);

typedef struct Asynx_TASKBODY() AsynxGenericTask;

#define Asynx_STATE(...) \
	typedef STRUCTDECL(Asynx_TOK(_Task)); \
	struct Asynx_TOK(_Task) Asynx_TASKBODY(__VA_ARGS__);

#define Asynx_LABEL(lbl) \
	[[fallthrough]]; case Asynx_TOK(_label_##lbl): \
	[[maybe_unused]] asynx_label_##lbl##__:

#define Asynx_SELF(lbl) \
	AsyncTask_upcast(asynx_data__, Asynx_makestate(asynx_fork_id__, Asynx_TOK(_label_##lbl)))

#define Asynx_LOCAL_THIS [[maybe_unused]] auto const this = asynx_data__;

#ifndef Asynx_LOCALS
	#define Asynx_LOCALS
#endif

#define Asynx_BEGIN \
	AsyncIntent Asynx_TOK(_entry)( \
		Ptr const asynx_vdata__, AsyncRT *const asynx_rt__, \
		AsyncTaskArgs const asynx_args__, AsyncResult *asynx_result__ \
	) { \
		[[maybe_unused]] \
		union { \
			AlcRes alc; \
			AsynxForkErr fork; \
		} asynx_error__; \
		auto const asynx_state__ = AsyncTaskArgs_state(asynx_args__); \
		[[maybe_unused]] \
		auto const asynx_context__ = AsyncTaskArgs_context(asynx_args__); \
		auto const asynx_label__ = Asynx_getlabel(asynx_state__); \
		[[maybe_unused]] \
		auto asynx_fork_id___ = Asynx_getforkid(asynx_state__); \
		auto const asynx_data__ = (Asynx_TOK(_Task)*)asynx_vdata__; \
		Asynx_LOCALS \
		switch (asynx_label__) { \
			case 0:

#define Asynx_RETURN_JOIN_SYNC { \
	auto const data = (AsynxGenericTask*)AsyncTask_data(asynx_data__->return_task__); \
	if (atom_sub(&data->fork_sync__, 1, atom_sync) != 1) \
		return AsyncIntent_Yield; \
}


#define Asynx_END \
		} \
		[[maybe_unused]] \
		asynx_return__:; \
		switch (asynx_data__->return__) { \
			case AsynxReturn_None: \
				return AsyncIntent_Yield; \
			case AsynxReturn_JoinResume: \
				Asynx_RETURN_JOIN_SYNC \
				[[fallthrough]]; \
			case AsynxReturn_Resume: \
				asynx_result__->task = asynx_data__->return_task__; \
				return AsyncIntent_Resume; \
			case AsynxReturn_JoinSuspend: \
				Asynx_RETURN_JOIN_SYNC \
				[[fallthrough]]; \
			case AsynxReturn_Suspend: \
				asynx_result__->task = asynx_data__->return_task__; \
				return AsyncIntent_Suspend; \
			case AsynxReturn_Resolve: \
				return AsyncFuture_resolve(asynx_data__->return_future__, asynx_result__); \
		} \
		UNREACHABLE; \
		asynx_catch_fork:; \
		asynx_catch_alc:; \
	}
