
constexpr AsyncTaskState Asynx_nullstate = 0;

#ifndef AsynxLabel_WIDTH
	#define AsynxLabel_WIDTH 8
#endif

#if AsynxLabel_WIDTH > 64
	#error "AsynxLabel_WIDTH > 64"
#elif AsynxLabel_WIDTH > 32
	typedef u64 AsynxLabel;
#elif AsynxLabel_WIDTH > 16
	typedef u32 AsynxLabel;
#elif AsynxLabel_WIDTH > 8
	typedef u16 AsynxLabel;
#else
	typedef u8 AsynxLabel;
#endif

#ifndef AsynxForkId_WIDTH
	#define AsynxForkId_WIDTH 8
#endif

#if AsynxForkId_WIDTH > 64
	#error "AsynxForkId_WIDTH > 64"
#elif AsynxForkId_WIDTH > 32
	typedef u64 AsynxForkId;
	typedef _Atomic u64_lf AsynxForkSync;
#elif AsynxForkId_WIDTH > 16
	typedef u32 AsynxForkId;
	typedef _Atomic u32_lf AsynxForkSync;
#elif AsynxForkId_WIDTH > 8
	typedef u16 AsynxForkId;
	typedef _Atomic u16_lf AsynxForkSync;
#else
	typedef u8 AsynxForkId;
	typedef _Atomic u8_lf AsynxForkSync;
#endif

constexpr auto AsynxForkId_width = AsynxForkId_WIDTH;
constexpr auto AsynxLabel_width = AsynxLabel_WIDTH;

enum {
	FIELD_DEF(AsynxLabel, AsynxLabel_width),
	FIELD_DEF(AsynxForkId, AsynxForkId_width),
	Asynx_statewidth
};

static_assert(Asynx_statewidth <= AsyncTaskState_width);

constexpr AsynxForkId AsynxForkId_max = FIELD_MAX(AsynxForkId);
constexpr AsyncTaskState AsynxForkId_one = FIELD_SET(AsynxForkId, 1);


AsyncTaskState Asynx_makestate(AsynxLabel label, AsynxForkId fork_id) {
	return (AsyncTaskState)(
		FIELD_SET(AsynxLabel, label),
		FIELD_SET(AsynxForkId, fork_id)
	);
}

AsynxForkId Asynx_getforkid(AsyncTaskState state) {
	return FIELD_GET(AsynxForkId, state);
}

AsynxLabel Asynx_getlabel(AsyncTaskState state) {
	return FIELD_GET(AsynxLabel, state);
}

typedef enum : u8 {
	AsynxReturn_None,
	AsynxReturn_Resume,
	AsynxReturn_Suspend,
	AsynxReturn_Resolve,
	AsynxReturn_JoinResume,
	AsynxReturn_JoinSuspend
} AsynxReturn;

#define Asynx_TASKBODY(align, ...) \
	{ \
		union { \
			_Alignas(align) char align__; \
			struct { \
				AsyncFn entry__; \
				union { \
					AsyncTask return_task__; \
					AsyncFuture *return_future__; \
				}; \
				AsynxForkSync fork_sync__; \
				AsynxReturn return__; \
				__VA_ARGS__ \
			}; \
		}; \
	}
