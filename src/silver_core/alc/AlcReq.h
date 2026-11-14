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
auto constexpr AlcAlignBits_width = ENUM_VALUE_WIDTH(ualign_width - 1);

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
usize constexpr Alc_maxalign = 1ull << FIELD_MAX(AlcAlignBits);

AlcReq AlcReq_align(ualign align) {
	#if Alc_SAFE
		// ensure alignment is power of 2
		ASSERT(
			(align & (align - 1)) == 0
		, "alignment is not power of 2");
		ASSERT(align <= Alc_maxalign);
	#endif

	return FIELD_SET(AlcAlignBits, __builtin_ctzg(align));
}

ualign AlcReq_getalign(AlcReq req) {
	return (ualign)(1ull << FIELD_GET(AlcAlignBits, req));
}

#define AlcReq_ALIGN(T) AlcReq_align(_Alignof(T))

AlcReq AlcReq_size(usize size) {
	#if Alc_SAFE
		ASSERT(size <= AlcSize_max);
	#endif

	return FIELD_SET(AlcSize, size);
}

AlcReq constexpr AlcReq_delete = FIELD(AlcIntent, Delete);
AlcReq constexpr AlcReq_zero = FLAG(AlcFlag_Zero);

AlcReq AlcReq_new(usize size, ualign align, bool zero) {
	return
		FIELD(AlcIntent, New) |
		AlcReq_size(size) |
		AlcReq_align(align) |
		(zero ? 0 : AlcReq_zero)
	;
}

AlcReq AlcReq_resize(usize size, ualign align, bool zero) {
	return
		FIELD(AlcIntent, Resize) |
		AlcReq_size(size) |
		AlcReq_align(align) |
		(zero ? 0 : AlcReq_zero)
	;
}
