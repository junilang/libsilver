typedef struct {
	u64 capacity;
	u64 size;
	AsyncTask tasks[];
} WeaverQueue;

usize ZZWeaverQueue_allocsize(usize capacity) {
	#if Weaver_SAFE
		return usize_pflx(offsetof(WeaverQueue, tasks), sizeof(AsyncTask), capacity);
	#else
		return offsetof(WeaverQueue, tasks) + sizeof(AsyncTask) * capacity;
	#endif
}

usize WeaverQueue_nextcapacity(usize capacity, usize target) {
	while (capacity < target) {
		capacity += capacity / 2;
	}

	return capacity;
}

#define WeaverQinfo_BITS 64

#define WeaverQinfo_INDEX_BITS 12
#define WeaverQinfo_INDEX_MASK ((1ull << WeaverQinfo_INDEX_BITS) - 1)
#define WeaverQinfo_INDEX_BIT (1ull)

#define WeaverQinfo_LOCK_SHIFT WeaverQinfo_INDEX_BITS
#define WeaverQinfo_LOCK_BITS 2
#define WeaverQinfo_LOCK_MASK (((1ull << WeaverQinfo_LOCK_BITS) - 1) << WeaverQinfo_LOCK_SHIFT)

#define WeaverQinfo_SWAP_LOCK (1ull << (WeaverQinfo_LOCK_SHIFT + 0))
#define WeaverQinfo_RESIZE_LOCK  (1ull << (WeaverQinfo_LOCK_SHIFT + 1))

#define WeaverQinfo_HEADER_BITS (WeaverQinfo_INDEX_BITS + WeaverQinfo_LOCK_BITS)

#define WeaverQinfo_RC_SHIFT WeaverQinfo_HEADER_BITS
#define WeaverQinfo_RC_BITS 8
#define WeaverQinfo_RC_MASK (((1ull << WeaverQinfo_RC_BITS) - 1) << WeaverQinfo_RC_SHIFT)
#define WeaverQinfo_RC_ONE (1ull << WeaverQinfo_RC_SHIFT)

#define WeaverQinfo_POS_SHIFT (WeaverQinfo_RC_BITS + WeaverQinfo_HEADER_BITS)
#define WeaverQinfo_POS_BITS ((WeaverQinfo_BITS - WeaverQinfo_POS_SHIFT))
#define WeaverQinfo_POS_ONE (1ull << WeaverQinfo_POS_SHIFT)
#define WeaverQinfo_POS_MAX ((1ull << WeaverQinfo_POS_BITS) - 1)
#define WeaverQinfo_POS_MASK (WeaverQinfo_POS_MAX << WeaverQinfo_POS_SHIFT)

typedef u64 WeaverQinfo;

usize WeaverQinfo_pos(WeaverQinfo this) {
	return (this & WeaverQinfo_POS_MASK) >> WeaverQinfo_POS_SHIFT;
}
