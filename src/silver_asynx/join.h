#define Asynx_JOIN_SYNC { \
	if (atom_sub(&asynx_data__->fork_sync__, 1, atom_sync) != 1) \
		return AsyncIntent_Yield; \
}

#define Asynx_JOIN_IMMEDIATE_TO(lbl) { \
	Asynx_JOIN_SYNC \
	asynx_fork_id__ = 0; \
	goto asynx_label_##lbl##__; \
}

#define Asynx_JOIN_IMMEDIATE(lbl) \
	Asynx_JOIN_IMMEDIATE_TO(lbl) \
	Asynx_LABEL(lbl)

#define Asynx_JOIN_RESUME_TO(lbl) { \
	Asynx_JOIN_SYNC \
	*asynx_out_task__ = AsyncTask_upcast( \
		asynx_data__, Asynx_makestate(0, Asynx_TOK(_Label_##lbl)) \
	); \
	return AsyncIntent_Resume; \
}

#define Asynx_JOIN_RESUME(lbl) \
	Asynx_JOIN_RESUME_TO(lbl) \
	Asynx_LABEL(lbl)

#define Asynx_JOIN_SUSPEND_TO(lbl) { \
	Asynx_JOIN_SYNC \
	*asynx_out_task__ = AsyncTask_upcast( \
		asynx_data__, Asynx_makestate(0, Asynx_TOK(_Label_##lbl)) \
	); \
	return AsyncIntent_Suspend; \
}

#define Asynx_JOIN_SUSPEND(lbl) \
	Asynx_JOIN_SUSPEND_TO(lbl) \
	Asynx_LABEL(lbl)
