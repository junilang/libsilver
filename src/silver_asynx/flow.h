#define Asynx_YIELD \
	return AsyncIntent_YIELD

#define Asynx_RETURN \
	goto asynx_return__

#define Asynx_GOTO(lbl) \
	goto asynx_label_##lbl##__

#define Asynx_RESUME_TO(lbl) { \
	*asynx_out_task__ = Asynx_SELF(lbl); \
	return AsyncIntent_Resume; \
}

#define Asynx_RESUME(lbl) \
	Asynx_RESUME_TO(lbl) \
	Asynx_LABEL(lbl)

#define Asynx_SUSPEND_TO(lbl) { \
	*asynx_out_task__ = Asynx_SELF(lbl); \
	return AsyncIntent_Suspend; \
}

#define Asynx_SUSPEND(lbl) \
	Asynx_SUSPEND_TO(lbl) \
	Asynx_LABEL(lbl)
