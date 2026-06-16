typedef enum : u8 {
	AlcAlign_1B,
	AlcAlign_2B,
	AlcAlign_4B,
	AlcAlign_8B,
	AlcAlign_16B,
	AlcAlign_32B,
	AlcAlign_64B,
	AlcAlign_128B,
	AlcAlign_256B,
	AlcAlign_512B,
	AlcAlign_1024B,
	AlcAlign_MAX
} AlcAlign;

ualign AlcAlign_get(AlcAlign align) {
	return (ualign)(1u << align);
}

// align must be power of 2
AlcAlign AlcAlign_set(ualign align) {
	return (AlcAlign)__builtin_ctzg(align, 0);
}

typedef enum : u8 {
	AlcRelative_None,
	AlcRelative_Local,
	AlcRelative_S16,
	AlcRelative_U16,
	AlcRelative_S32,
	AlcRelative_U32,
	AlcRelative_MAX,
} AlcRelative;

typedef enum : u8 {
	AlcIntent_Least,
	AlcIntent_Exact,
	AlcIntent_Range,
	AlcIntent_Loose,
	AlcIntent_MAX
} AlcIntent;

typedef u64 AlcSize;
constexpr u8 AlcSize_width = 36; // supports at most 64GB allocations
constexpr AlcSize AlcSize_max = (1ull << AlcSize_width) - 1;

typedef u8 AlcOffersSize;
constexpr AlcOffersSize Alc_ResolveNone = u8_max;

typedef u64 AlcReq; enum {
	FIELD_DEFINE(AlcSize, AlcSize_width),
	FIELD_DEFINE(AlcAlign, 4),
	FIELD_DEFINE(AlcRelative, 3),
	FIELD_DEFINE(AlcIntent, 2),
	FIELD_DEFINE(AlcOffersSize, 3), // up to 7 offers
	AlcReq_BIT_Zero,
	AlcReq_BIT_EmbedSize,
	AlcReq_BIT_Promise,
	AlcReq_END
};

static_assert(AlcReq_END <= 64);
static_assert(FIELD_MAX(AlcAlign) >= AlcAlign_MAX - 1);
static_assert(FIELD_MAX(AlcRelative) >= AlcRelative_MAX - 1);
static_assert(FIELD_MAX(AlcIntent) >= AlcIntent_MAX - 1);
