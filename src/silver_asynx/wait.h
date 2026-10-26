#define Asynx_WAIT_HEADER(lbl, future) \
	constexpr AsyncRes resolved__ = \
		FIELD(AsyncRes_Class, AsyncFutureRes) | \
		FIELD(AsyncRes_Origin, AsyncFuture_SetTask) | \
		FIELD_SET(AsyncRes_Code, AsyncFutureRes_IsResolved) \
	; \
	auto const self__ = Asynx_SELF(lbl); \
	asynx_error__.any = AsyncFuture_settask((future), self__); \
	if (asynx_error__.any == AsyncRes_Ok) \
		return AsyncIntent_Yield; \
	if (asynx_error__.any != resolved__) \
		goto asynx_catch_any__;


#define Asynx_WAIT_IMMEDIATE_TO(lbl, future) { \
	Asynx_WAIT_HEADER(lbl, (future)) \
	Asynx_GOTO(lbl); \
}

#define Asynx_WAIT_IMMEDIATE(lbl, future) \
	Asynx_WAIT_IMMEDIATE_TO(lbl, (future)); \
	Asynx_GOTO(lbl);

#define Asynx_WAIT_RESUME_TO(lbl, future) { \
	Asynx_WAIT_HEADER(lbl, (future)) \
	*asynx_out_task__ = self__; \
	return AsyncIntent_Resume; \
}

#define Asynx_WAIT_RESUME(lbl, future) \
	Asynx_WAIT_RESUME_TO(lbl, (future)); \
	Asynx_GOTO(lbl);

#define Asynx_WAIT_SUSPEND_TO(lbl, future) { \
	Asynx_WAIT_HEADER(lbl, (future)) \
	*asynx_out_task__ = self__; \
	return AsyncIntent_Suspend; \
}

#define Asynx_WAIT_SUSPEND(lbl, future) \
	Asynx_WAIT_SUSPEND_TO(lbl, (future)); \
	Asynx_GOTO(lbl);
