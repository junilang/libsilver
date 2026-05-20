typedef struct {
	u64 capacity;
	u64 size;
	AsyncTask tasks[];
} WeaverQueue;

usize ZZWeaverQueue_allocsize(usize capacity) {
	// TODO add unsafe version
	return usize_pflx(offsetof(WeaverQueue, tasks), sizeof(AsyncTask), capacity);
}

usize WeaverQueue_nextcapacity(usize capacity, usize target) {
	while (capacity < target) {
		capacity += capacity / 2;
	}

	return capacity;
}

// WeaverQinfo:
// 	3 bit index
// 	1 bit lock
//    16 bit refcount
//    44 bit position

#define WeaverQinfo_BITS 64

#define WeaverQinfo_INDEX_BITS 3
#define WeaverQinfo_INDEX_MASK ((1ull << WeaverQinfo_INDEX_BITS) - 1)
#define WeaverQinfo_INDEX_BIT (1ull)

#define WeaverQinfo_LOCK_BIT (1ull << WeaverQinfo_INDEX_BITS)

#define WeaverQinfo_HEADER_BITS (WeaverQinfo_INDEX_BITS + 1)

#define WeaverQinfo_RC_SHIFT WeaverQinfo_HEADER_BITS
#define WeaverQinfo_RC_BITS 16
#define WeaverQinfo_RC_MASK (((1ull << WeaverQinfo_RC_BITS) - 1) << WeaverQinfo_HEADER_BITS)
#define WeaverQinfo_RC_ONE (1ull << WeaverQinfo_RC_SHIFT)


#define WeaverQinfo_POS_SHIFT (WeaverQinfo_RC_BITS + WeaverQinfo_HEADER_BITS)
#define WeaverQinfo_POS_BITS ((WeaverQinfo_BITS - WeaverQinfo_POS_SHIFT))
#define WeaverQinfo_POS_ONE (1ull << WeaverQinfo_POS_SHIFT)
#define WeaverQinfo_POS_MAX ((1ull << WeaverQinfo_POS_BITS))
#define WeaverQinfo_POS_MASK (WeaverQinfo_POS_MAX << WeaverQinfo_POS_SHIFT)

typedef u64 WeaverQinfo;

WeaverQinfo WeaverQinfo_index(WeaverQinfo this) {
	return this & WeaverQinfo_INDEX_MASK;
}

WeaverQinfo WeaverQinfo_lock(WeaverQinfo this) {
	return this & WeaverQinfo_LOCK_BIT;
}

WeaverQinfo WeaverQinfo_maskpos(WeaverQinfo this) {
	return this & WeaverQinfo_POS_MASK;
}

WeaverQinfo WeaverQinfo_pos(WeaverQinfo this) {
	return this >> WeaverQinfo_POS_SHIFT;
}

WeaverQinfo WeaverQinfo_maskrc(WeaverQinfo this) {
	return this & WeaverQinfo_RC_MASK;
}

WeaverQinfo WeaverQinfo_rc(WeaverQinfo this) {
	return (this & WeaverQinfo_RC_MASK) >> WeaverQinfo_RC_SHIFT;
}

void WeaverQinfo_xprint(WeaverQinfo this, OutStream os) {
	PRINT(os,
		WeaverQinfo_lock(this) ? "L" : "",
		WeaverQinfo_index(this), ",",
		WeaverQinfo_rc(this),",",
		WeaverQinfo_pos(this)
	);
}

void WeaverQinfo_print(const WeaverQinfo *this, OutStream os) {
	WeaverQinfo_xprint(*this, os);
}

IPrintable_GENERATE(WeaverQinfo, const WeaverQinfo*)
