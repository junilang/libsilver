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
	X(ErrUnknown) \
	X(Aux)

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
		return (AlcRes)err;
	}
	return AlcRes_Ok;
}

static void AlcRes_UNWRAP_internal(AlcRes res, ConstPtr func_name, ConstPtr panic_header) {
	if (!res) return;

	ubyte buf[8];
	ConstPtr str;
	if (res < AlcRes_Aux) {
		str = AlcRes_repr[res].data;
	} else {
		auto const r =
			ullong_tostr(res, FLAGS(IntFmt, ZeroOffset, NullTerminate), buf, 8)
		;

		if (r < 0)
			str = "tostr error :)";
		else
			str = buf;
	}

	PANIC_internal(func_name, panic_header, str, nullptr);
}

#define AlcRes_UNWRAP(result) \
	AlcRes_UNWRAP_internal(result, __func__, PANIC_IDENTIFIER" AlcRes_UNWRAP: res=")


static Ptr AlcPtr_UNWRAP_internal(AlcPtr result, ConstPtr func_name, ConstPtr panic_header) {
	auto const res = AlcPtr_get(result);
	AlcRes_UNWRAP_internal(res, func_name, panic_header);
	return (Ptr)result;
}

#define AlcPtr_UNWRAP(result) \
	AlcPtr_UNWRAP_internal(result, __func__, PANIC_IDENTIFIER" AlcPtr_UNWRAP:")

typedef struct {
	usize size;
	usize data[3];
} AlcPromise;
