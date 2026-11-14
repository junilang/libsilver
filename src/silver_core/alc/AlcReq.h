#ifndef AlcSize_WIDTH
	#define AlcSize_WIDTH 48
#endif

#if SIZE_WIDTH < AlcSize_WIDTH
	#undef AlcSize_WIDTH
	#define AlcSize_WIDTH SIZE_WIDTH
#endif

auto constexpr AlcSize_width = AlcSize_WIDTH;

ENUM_GENERATE(AlcIntent, u8,
	New, Resize, Delete, Attr, Lock, Unlock
)

auto constexpr AlcIntent_width = ENUM_WIDTH(AlcIntent);
auto constexpr AlcAlignBits_width = ENUM_VALUE_WIDTH(15);

typedef u64 AlcReq; enum {
	FIELD_DEF(AlcSize, AlcSize_width),
	FIELD_DEF(AlcAlignBits, AlcAlignBits_width),
	FIELD_DEF(AlcIntent, AlcIntent_width),
	FLAGS_DEF(AlcFlag,
		Aux, Zero, Locked, NoWait
	),
	AlcReq_width
};

static_assert(AlcReq_width <= 64);

usize constexpr AlcSize_max = FIELD_MAX(AlcSize);

/*
typedef struct {
	usize size;
	ConstPtr hint;
	ualign align;
	AlcIntent intent;
	AlcRelative relative;
	AlcRange range;
	AlcFlag flags;
	u8 offers_size;
} AlcReq;

typedef enum : u8 {
	AlcRelative_None,
	AlcRelative_Local,
	AlcRelative_S16,
	AlcRelative_U16,
	AlcRelative_S32,
	AlcRelative_U32,
} AlcRelative;

typedef enum : u8 {
	AlcRange_Least,
	AlcRange_Exact,
	AlcRange_Range,
	AlcRange_Loose,
} AlcRange;

*/

typedef u32 AlcAttr; enum {
	FLAGS_DEF(AlcAttr,
		IntentNew,
		IntentResize,
		IntentDelete,
		IntentLock,
		IntentUnlock,
		FeatureZero,
		Concurrent
	),
	AlcAttr_width
};

static_assert(AlcAttr_width <= 32);
