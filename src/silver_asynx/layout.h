typedef enum : u8 {
	AsynxReturn_None,
	AsynxReturn_Resume,
	AsynxReturn_Suspend,
	AsynxReturn_Resolve,
	AsynxReturn_JoinResume,
	AsynxReturn_JoinSuspend
} AsynxReturn;

constexpr AsyncTaskState Asynx_nullstate = 0;

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

constexpr u8 AsynxForkId_width = AsynxForkId_WIDTH;
constexpr AsynxForkId AsynxForkId_mask = (AsynxForkId)(u64_max >> (64 - AsynxForkId_width));
constexpr AsynxForkId AsynxForkId_max = AsynxForkId_mask;

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

constexpr u8 AsynxLabel_width = AsynxLabel_WIDTH;
constexpr AsynxLabel AsynxLabel_max = (AsynxLabel)(~0ull);

constexpr AsyncTaskState AsynxForkId_one = 1ull << AsynxLabel_width;

AsyncTaskState Asynx_makestate(AsynxForkId fork_id, AsynxLabel label) {
	return (AsyncTaskState)(
		((AsyncTaskState)(fork_id) << AsynxLabel_width) | (AsyncTaskState)label
	);
}

AsynxForkId Asynx_getforkid(AsyncTaskState state) {
	return (AsynxForkId)(state >> AsynxLabel_width);
}

AsynxForkId Asynx_getlabel(AsyncTaskState state) {
	return (AsynxForkId)(state & AsynxLabel_max);
}

#define Asynx_TASKBODY(align, ...) \
	{ \
		union { \
			_Alignas((align ? align : cpu_hdi_size)) char align__; \
			struct { \
				AsyncFn entry__; \
				union { \
					AsyncTask return_task__; \
					AsyncFuture *return_future__; \
				}; \
				AsynxForkSync fork_sync__; \
				AsynxForkId fork_count__; \
				AsynxReturn return__; \
				__VA_ARGS__ \
			}; \
		}; \
	}
