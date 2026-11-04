#define XS \
	X(Ok) \
	X(Locked) \
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

[[noreturn, gnu::noinline]]
void AlcRes_panic_(AlcRes res, ConstPtr func_name, ConstPtr header) {
	// TODO print res value
	String msg[] = {
		String_INIT(func_name),
		String_INIT(header)
	};
	panic_msg_(msg, COUNTOF(msg));
}

void AlcRes_assert_(AlcRes res, ConstPtr func_name, ConstPtr header) {
	if (res) AlcRes_panic_(res, func_name, header);
}

Ptr AlcPtr_unwrap_(AlcPtr ptr, ConstPtr func_name, ConstPtr header) {
	auto const res = AlcPtr_get(ptr);
	if (res) AlcRes_panic_(res, func_name, header);
	return (Ptr)ptr;
}

#define AlcRes_ASSERT(result) \
	AlcRes_assert_(result, __func__, SOURCE_IDENTIFIER " AlcRes_ASSERT(" #result "):")


#define AlcPtr_UNWRAP(result) \
	AlcPtr_unwrap_(result, __func__, SOURCE_IDENTIFIER " AlcPtr_UNWRAP(" #result "):")

typedef struct {
	usize size;
	usize data[3];
} AlcPromise;
