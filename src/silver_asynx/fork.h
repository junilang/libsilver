#define XS \
	X(Ok) \
	X(ErrRecursive) \
	X(ErrOverflow) \
	X(ErrZero) \
	X(ErrOne)

	typedef enum : u8 {
		#define X(N) AsynxForkRes_##N,
			XS
		#undef X
	} AsynxForkRes;

	const String AsynxForkRes_repr[] = {
		#define X(N) [AsynxForkRes_##N] = String_INIT(#N),
			XS
		#undef X
	};
#undef XS

[[noreturn, gnu::noinline]]
void AsynxForkRes_panic_(AsynxForkRes res, ConstPtr func_name, ConstPtr header) {
	String msg[] = {
		String_INIT(func_name),
		String_INIT(header),
		AsynxForkRes_repr[res]
	};
	panic_msg_(msg, COUNTOF(msg));
}

AsynxForkRes Asynx_forkvalidate(AsynxForkId id, usize n) {
	#if Asynx_SAFE
		if (id != 0)
			return AsynxForkRes_ErrRecursive;

		if (n > AsynxForkId_max)
			return AsynxForkRes_ErrOverflow;

		if (n == 0)
			return AsynxForkRes_ErrZero;

		if (n == 1)
			return AsynxForkRes_ErrOne;
	#endif

	return AsynxForkRes_Ok;
}

// size of stack array in Asynx_fork function
constexpr usize Asynx_fsasize = Asynx_FSASIZE;

AsyncRes Asynx_fork(AsyncRT *rt, Ptr data, AsynxForkId count, AsyncTaskState state) {
	AsyncTask tasks[Asynx_fsasize];
	do {
		AsynxForkId const n = count < Asynx_fsasize ? count : Asynx_fsasize;
		for (AsynxForkId i = 0; i < n; i++) {
			tasks[i] = AsyncTask_upcast(data, state);
			state += AsynxForkId_one;
		}
		count -= n;

		auto res = AsyncRT_submit(rt, tasks, n);
		if (res) return res;
	} while (count);

	return AlcRes_Ok;
}

#define Asynx_SETFORK(data, n) { \
	(data)->fork_sync__ = n; \
}

#define Asynx_FORK_COMMON(n_) \
	asynx_error__.fork = Asynx_forkvalidate(asynx_fork_id__, (n_)); \
	if (asynx_error__.fork) goto asynx_catch_fork__;\
	auto const n = (AsynxForkId)(n_); \
	Asynx_SETFORK(asynx_data__, n); \

#define Asynx_FORK_IMMEDIATE_TO(lbl, n_) { \
	Asynx_FORK_COMMON(n_) \
	auto const state = Asynx_makestate(Asynx_TOK(_label_##lbl), 0); \
	asynx_error__.any = Asynx_fork(asynx_rt__, asynx_data__, n - 1, state); \
	if (asynx_error__.any) goto asynx_catch_any__; \
	asynx_fork_id__ = 1; \
	goto asynx_label_##lbl##__; \
}

#define Asynx_FORK_IMMEDIATE(lbl, n_) \
	Asynx_FORK_IMMEDIATE_TO(lbl, n_) \
	Asynx_LABEL(lbl) \

#define Asynx_FORK_RESUME_TO(lbl, n_) { \
	Asynx_FORK_COMMON(n_) \
	auto const state = Asynx_makestate(Asynx_TOK(_label##lbl), 1); \
	asynx_error__.any = Asynx_fork(asynx_rt__, asynx_data__, n - 1, state + AsynxForkId_one); \
	if (asynx_error__.any) goto asynx_catch_any__; \
	*asynx_out_task__ = AsyncTask_upcast(asynx_data__, state); \
	return AsyncIntent_Resume; \
}

#define Asynx_FORK_RESUME(lbl, n_) \
	Asynx_FORK_RESUME_TO(lbl, n_) \
	Asynx_LABEL(lbl) \

#define Asynx_FORK_SUSPEND_TO(lbl, n_) { \
	Asynx_FORK_COMMON(n_) \
	auto const state = Asynx_makestate(Asynx_TOK(_label_##lbl), 1); \
	asynx_error__.any = Asynx_fork(asynx_rt__, asynx_data__, n, state); \
	if (asynx_error__.any) goto asynx_catch_any__; \
	return AsyncIntent_Yield; \
}

#define Asynx_FORK_SUSPEND(lbl, n_) \
	Asynx_FORK_SUSPEND_TO(lbl, n_) \
	Asynx_LABEL(lbl)
