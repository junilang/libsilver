#define Asynx_CALL_RESUME_TO(lbl, tsk) { \
	(tsk)->return_task__ = Asynx_SELF(lbl); \
	(tsk)->return__ = AsynxReturn_Resume; \
	*asynx_out_task__ = AsyncTask_upcast((tsk), Asynx_nullstate); \
	return AsyncIntent_Call; \
}

#define Async_CALL_RESUME(lbl, tsk) \
	Async_CALL_RESUME_TO(lbl, tsk) \
	Async_LABEL(lbl)

#define Asynx_CALL_SUSPEND_TO(lbl, tsk) { \
	(tsk)->return_task__ = Asynx_SELF(lbl); \
	(tsk)->return__ = AsynxReturn_Suspend; \
	*asynx_out_task__ = AsyncTask_upcast((tsk), Asynx_nullstate); \
	return AsyncIntent_Call; \
}

#define Async_CALL_SUSPEND(lbl, tsk) \
	Async_CALL_SUSPEND_TO(lbl, tsk) \
	Async_LABEL(lbl)

#define Asynx_CALL_JOIN_RESUME_TO(lbl, tsk) { \
	(tsk)->return_task__ = Asynx_SELF(lbl); \
	(tsk)->return__ = AsynxReturn_JoinResume; \
	*asynx_out_task__ = AsyncTask_upcast((tsk), Asynx_nullstate); \
	return AsyncIntent_Call; \
}

#define Async_CALL_JOIN_RESUME(lbl, tsk) \
	Async_CALL_JOIN_RESUME_TO(lbl, tsk) \
	Async_LABEL(lbl)

#define Asynx_CALL_JOIN_SUSPEND_TO(lbl, tsk) { \
	(tsk)->return_task__ = Asynx_SELF(lbl); \
	(tsk)->return__ = AsynxReturn_JoinSuspend; \
	*asynx_out_task__ = AsyncTask_upcast((tsk), Asynx_nullstate); \
	return AsyncIntent_Call; \
}

#define Async_CALL_JOIN_SUSPEND(lbl, tsk) \
	Async_CALL_JOIN_SUSPEND_TO(lbl, tsk) \
	Async_LABEL(lbl)
