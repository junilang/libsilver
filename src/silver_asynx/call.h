#define Asynx_CALL_RESUME_TO(lbl, tsk) { \
	(tsk)->return_task__ = Asynx_SELF(lbl); \
	(tsk)->return__ = AsynxReturn_Resume; \
	asynx_result__->task = AsyncTask_upcast((tsk), Asynx_nullstate); \
	return AsyncIntent_Call; \
}

#define Async_CALL_RESUME(lbl, tsk) \
	Async_CALL_RESUME_TO(lbl, tsk) \
	Async_LABEL(lbl)
