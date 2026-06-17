#define XS \
	X(Ok) \
	X(ErrInvalidSize) \
	X(ErrInvalidAlign) \
	X(ErrInvalidRelative) \
	X(ErrInvalidFlags) \
	X(ErrInvalidRange) \
	X(ErrInternal) \
	X(ErrNoMemory) \
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

typedef struct {
	usize size;
	usize data[3];
} AlcPromise;
