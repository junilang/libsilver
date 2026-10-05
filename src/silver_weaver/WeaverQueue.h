typedef union {
	_Alignas(cpu_hdi_size) char align__;
	struct {
		usize capacity;
		usize size;
		AsyncTask *tasks;
	};
} WeaverQueue;

typedef _Atomic u64_lf WeaverQinfo_Atomic;

typedef u64 WeaverQinfo; enum {
	FIELD_DEF(WeaverQinfo_Index, 12),
	FIELD_DEF(WeaverQinfo_Lock, 2),
	FIELD_DEF(WeaverQinfo_Rc, 8),
	FIELD_DEF_UNTIL(WeaverQinfo_Pos, 64)
};

enum {
	WeaverQinfo_Lock_None,
	WeaverQinfo_Lock_Swap,
	WeaverQinfo_Lock_Resize,
};
