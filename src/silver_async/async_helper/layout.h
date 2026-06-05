#ifndef async_ForkId_BITS
	// allows for 255 forks
	#define async_ForkId_BITS 8
#endif

#if async_ForkId_BITS > 64
	#error "async_ForkId_BITS > 64"
#elif async_ForkId_BITS > 32
	#define async_ForkId_TYPE u64
#elif async_ForkId_BITS > 16
	#define async_ForkId_TYPE u32
#elif async_ForkId_BITS > 8
	#define async_ForkId_TYPE u16
#else
	#define async_ForkId_TYPE u8
#endif

#ifndef async_Label_BITS
	// allows for 255 labels
	#define async_Label_BITS 8
#endif

#if async_Label_BITS > 64
	#error "async_Label_BITS > 64"
#elif async_Label_BITS > 32
	#define async_Label_TYPE u64
#elif async_Label_BITS > 16
	#define async_Label_TYPE u32
#elif async_Label_BITS > 8
	#define async_Label_TYPE u16
#else
	#define async_Label_TYPE u8
#endif

#if (async_Label_BITS + async_ForkId_BITS) > AsyncTaskState_BITS
	#error "async: AsyncTaskState too small"
#endif

#define async_ForkId_MAX ((1ull << async_ForkId_BITS) - 1)
#define async_Label_MAX ((1ull << async_Label_BITS) - 1)

#if (CHAR_WIDTH >= async_ForkId_BITS) && (ATOMIC_CHAR_LOCK_FREE == 2)
	#define async_ForkSync_TYPE unsigned char
#elif (SHRT_WIDTH >= async_ForkId_BITS) && (ATOMIC_SHORT_LOCK_FREE == 2)
	#define async_ForkSync_TYPE unsigned short
#elif (INT_WIDTH >= async_ForkId_BITS) && (ATOMIC_INT_LOCK_FREE == 2)
	#define async_ForkSync_TYPE unsigned int
#elif (LONG_WIDTH >= async_ForkId_BITS) && (ATOMIC_LONG_LOCK_FREE == 2)
	#define async_ForkSync_TYPE unsigned long
#elif (LLONG_WIDTH >= async_ForkId_BITS) && (ATOMIC_LLONG_LOCK_FREE == 2)
	#define async_ForkSync_TYPE unsigned long long
#else
	#error "no suitable lock-free atomic integer type for fork synchronization"
#endif

#define async_NULL_TASK_STATE 0

#define async_Return_NONE 0
#define async_Return_RESUME 1
#define async_Return_SUSPEND 2
#define async_Return_RESOLVE 3
#define async_Return_JOIN_RESUME 4
#define async_Return_JOIN_SUSPEND 5

#define async_Return_TYPE u8

#define async_TASK_STRUCT_BODY(...) \
	{ \
		AsyncFn entry__; \
		union { \
			AsyncTask return_task__; \
			AsyncFuture return_future__; \
			struct { \
				Ptr return_join_data__; \
				async_Label_TYPE return_join_label__; \
			}; \
		}; \
		_Atomic async_ForkSync_TYPE fork_sync__; \
		async_ForkId_TYPE fork_count__; \
		async_Return_TYPE return__; \
		__VA_ARGS__ \
	}

#define async_MAKE_TASK_STATE(fid, lbl) \
	((AsyncTaskState)((AsyncTaskState)(fid) << async_Label_BITS) | (lbl))

#define async_GET_FORK_ID(state) \
	((async_ForkId_TYPE)((state) >> async_Label_BITS))

#define async_GET_LABEL(state) \
	((async_Label_TYPE)((state) & async_Label_MAX))
