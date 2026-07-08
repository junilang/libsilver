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

static Ptr AlcPtr_UNWRAP_internal(AlcPtr result, ConstPtr func_name, ConstPtr panic_header) {
	auto res = AlcPtr_get(result);
	if (res) PANIC_internal(func_name, panic_header, AlcRes_repr[res].data, nullptr);
	return (Ptr)result;
}

#define AlcPtr_UNWRAP(result) \
	AlcPtr_UNWRAP_internal(result, __func__, PANIC_IDENTIFIER" AlcPtr_UNWRAP:")

static void AlcRes_UNWRAP_internal(AlcRes res, ConstPtr func_name, ConstPtr panic_header) {
	if (res) PANIC_internal(func_name, panic_header, AlcRes_repr[res].data, nullptr);
}

#define AlcRes_UNWRAP(result) \
	AlcRes_UNWRAP_internal(result, __func__, PANIC_IDENTIFIER" AlcRes_UNWRAP:")

typedef struct {
	usize size;
	usize data[3];
} AlcPromise;
