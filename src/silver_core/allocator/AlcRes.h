#define XS \
	X(Ok) \
	X(ErrInvalidSize) \
	X(ErrInvalidAlign) \
	X(ErrInvalidRelative) \
	X(ErrInvalidFlags) \
	X(ErrInvalidRange) \
	X(ErrInvalidMem) \
	X(ErrOverflow) \
	X(ErrInternal) \
	X(ErrNoMemory) \
	X(ErrUnsupported) \
	X(ErrUnimplemented) \
	X(ErrUnknown)

typedef enum : u8 {
	#define X(N) AlcRes_##N,
		XS
	#undef X
} AlcRes;

const String AlcRes_repr[] = {
	#define X(N) [AlcRes_##N] = String_INIT(#N),
		XS
	#undef X
};

#undef XS

typedef UNIQUEPTR(AlcPtr);

AlcPtr AlcPtr_set(AlcRes err) {
	return (AlcPtr)(-(isize)err);
}

bool AlcPtr_check(AlcPtr result) {
	isize c = (isize)result;
	if (c <= -1 && c >= -4095)
		return true;

	return false;
}

AlcRes AlcPtr_get(AlcPtr result) {
	isize c = (isize)result;
	if (c <= -1 && c >= -4095) {
		u16 err = (u16)(-c);
		if (err > AlcRes_ErrUnknown)
			return AlcRes_ErrUnknown;
		return (AlcRes)err;
	}
	return AlcRes_Ok;
}

AlcRes AlcPtr_unwrap(AlcPtr result, Ptr ptr) {
	auto res = AlcPtr_get(result);
	if (res)
		*(Ptr*)ptr = nullptr;
	else
		*(Ptr*)ptr = result;

	return res;
}

Ptr AlcPtr_panic(AlcPtr result) {
	if (AlcPtr_get(result)) PANIC();
	return (Ptr)result;
}

typedef struct {
	usize size;
	usize data[3];
} AlcPromise;
