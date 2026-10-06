typedef union {
	Weaver_CACHE_ALIGN char align__;
	struct {
		usize capacity;
		usize size;
		AsyncTask tasks[];
	};
} WeaverQueue;

typedef _Atomic u64_lf WeaverQueueInfo_Atomic;

typedef u64 WeaverQueueInfo; enum {
	FIELD_DEF(WeaverQueue_Ticker, 12),
	FLAG_DEF(WeaverQueue_LockSwap),
	FLAG_DEF(WeaverQueue_LockResize),
	FIELD_DEF(WeaverQueue_Rc, 8),
	FIELD_DEF_UNTIL(WeaverQueue_Pos, 64)
};

enum {
	WeaverQueue_Lock_None,
	WeaverQueue_Lock_Swap,
	WeaverQueue_Lock_Resize,
};

constexpr WeaverQueueInfo WeaverQueue_Ticker_one = FIELD_SET(WeaverQueue_Ticker, 1);
constexpr WeaverQueueInfo WeaverQueue_Ticker_mask = FIELD_MASK(WeaverQueue_Ticker);

constexpr WeaverQueueInfo WeaverQueue_Rc_one = FIELD_SET(WeaverQueue_Rc, 1);
constexpr WeaverQueueInfo WeaverQueue_Rc_mask = FIELD_MASK(WeaverQueue_Rc);

constexpr WeaverQueueInfo WeaverQueue_Pos_one = FIELD_SET(WeaverQueue_Pos, 1);
constexpr WeaverQueueInfo WeaverQueue_Pos_mask = FIELD_MASK(WeaverQueue_Pos);

constexpr WeaverQueueInfo WeaverQueue_LockSwap = FLAG(WeaverQueue_LockSwap);
constexpr WeaverQueueInfo WeaverQueue_LockResize = FLAG(WeaverQueue_LockResize);
