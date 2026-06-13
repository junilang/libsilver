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

typedef u64 AlcSize;
constexpr u8 AlcSize_width = 36; // supports at most 64GB allocations
constexpr AlcSize AlcSize_max = (1ull << AlcSize_width) - 1;

typedef u64 AlcReq; enum {
	FIELD_DEFINE(AlcSize, AlcSize_width),
	FIELD_DEFINE(AlcAlign, 4),
	FIELD_DEFINE(AlcRelative, 3),
	AlcReq_BIT_Zero,
	AlcReq_END
};

static_assert(AlcReq_END <= 64);
static_assert(FIELD_MAX(AlcAlign) >= AlcAlign_MAX);
static_assert(FIELD_MAX(AlcRelative) >= AlcRelative_MAX);

#define XS \
	X(Ok) \
	X(ErrInternal) \
	X(ErrTooLarge) \
	X(ErrNoMemory) \
	X(ErrInvalidSize) \
	X(ErrInvalidAlign) \
	X(ErrInvalidRelative) \
	X(ErrUnsupported) \
	X(ErrUnimplemented) \
	X(ErrUnknown)

typedef enum : u8 {
	#define X(N) AlcRes_##N,
		XS
	#undef X
	AlcRes_MAX
} AlcRes;

const String AlcRes_repr[] = {
	#define X(N) [AlcRes_##N] = STRING_INIT(#N),
		XS
	#undef X
};

Ptr AlcRes_set(AlcRes err) {
	return (Ptr)(-(isize)err);
}

AlcRes AlcRes_get(ConstPtr result) {
	isize c = (isize)result;
	if (c <= -1 && c >= -4095) {
		u16 err = (u16)(-c);
		if (err > AlcRes_ErrUnknown)
			return AlcRes_ErrUnknown;
		return (AlcRes)err;
	}
	return AlcRes_Ok;
}
