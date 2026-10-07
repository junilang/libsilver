typedef _Atomic u64_lf WeaverQueueInfo_Atomic;
typedef u64 WeaverQueueInfo; enum {
	FLAG_DEF(WeaverQueue_LockSwap),
	FLAG_DEF(WeaverQueue_LockResize),
	FIELD_DEF(WeaverQueue_Rc, WeaverThreadId_width),
	FIELD_DEF(WeaverQueue_Pos, 32)
};

enum {
	WeaverQueue_Lock_None,
	WeaverQueue_Lock_Swap,
	WeaverQueue_Lock_Resize,
};

constexpr WeaverQueueInfo WeaverQueue_Rc_one = FIELD_SET(WeaverQueue_Rc, 1);
constexpr WeaverQueueInfo WeaverQueue_Rc_mask = FIELD_MASK(WeaverQueue_Rc);
constexpr WeaverQueueInfo WeaverQueue_Rc_gtone =
	WeaverQueue_Rc_mask & (~WeaverQueue_Rc_one)
;


constexpr WeaverQueueInfo WeaverQueue_Pos_one = FIELD_SET(WeaverQueue_Pos, 1);
constexpr WeaverQueueInfo WeaverQueue_Pos_mask = FIELD_MASK(WeaverQueue_Pos);

//constexpr WeaverQueueInfo WeaverQueue_Empty = FLAG(WeaverQueue_Empty);
constexpr WeaverQueueInfo WeaverQueue_LockSwap = FLAG(WeaverQueue_LockSwap);
constexpr WeaverQueueInfo WeaverQueue_LockResize = FLAG(WeaverQueue_LockResize);

constexpr WeaverQueueInfo WeaverQueue_lockmask = FLAGS(WeaverQueue, LockSwap, LockResize);

// keep refcount while swapping
constexpr WeaverQueueInfo WeaverQueue_swapmask =
	FIELD_MASK(WeaverQueue_Rc)
;
