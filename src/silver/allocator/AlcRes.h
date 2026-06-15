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

#undef XS

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

typedef u64 AlcOffer; enum {
	AlcOffer_Size_END = AlcSize_END,
	FIELD_DEFINE(AlcRes, 4),
	FIELD_DEFINE(AlcOffer_Num, 4)
};

static_assert(FIELD_MAX(AlcRes) >= AlcRes_MAX - 1);
