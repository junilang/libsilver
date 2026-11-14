ENUM_GENERATE(AlcRes, u16,
	Ok,
	Locked,
	ErrInvalidSize,
	ErrInvalidAlign,
	ErrInvalidRelative,
	ErrInvalidFlags,
	ErrInvalidRange,
	ErrInvalidMem,
	ErrOverflow,
	ErrInternal,
	ErrNoMemory,
	ErrUnsupported,
	ErrUnimplemented,
	ErrUnknown,
	Aux
)

auto constexpr AlcRes_max = 4095;

typedef UNIQUEPTR(AlcPtr);

AlcPtr AlcPtr_set(AlcRes err) {
	#if Alc_SAFE
		ASSERT(err <= 4095);
	#endif

	return (AlcPtr)(-(isize)err);
}

bool AlcPtr_check(AlcPtr result) {
	isize c = (isize)result;
	if (c <= -1 && c >= -AlcRes_max)
		return true;

	return false;
}

AlcRes AlcPtr_get(AlcPtr result) {
	isize c = (isize)result;
	if (c <= -1 && c >= -AlcRes_max) {
		return (AlcRes)(-c);
	}
	return AlcRes_Ok;
}

PANIC_RES_GENERATE(AlcRes, AlcRes_str)

#define AlcRes_ASSERT(res) ASSERT_RES(AlcRes, res)

Ptr AlcPtr_unwrap_(AlcPtr ptr, ConstPtr func_name, ConstPtr header) {
	AlcRes_assert_(AlcPtr_get(ptr), func_name, header);
	return (Ptr)ptr;
}

#define AlcPtr_UNWRAP(res) \
	AlcPtr_unwrap_(res, __func__, SOURCE_IDENTIFIER " AlcPtr_UNWRAP(" #res "):")
