ENUM_GENERATE(AlcRes, u16,
	Ok,
	Locked,
	ErrInvalidSize,
	ErrInvalidAlign,
	ErrInvalidFlags,
	ErrInvalidAux,
	ErrInvalidMem,
	ErrOverflow,
	ErrNoMemory,
	ErrInternal,
	ErrUnsupported,
	ErrUnimplemented,
	ErrUnknown,
	Aux
)

auto constexpr AlcRes_max = 4095;

typedef UNIQUEPTR(AlcPtr);

AlcPtr constexpr AlcPtr_Ok = nullptr;

AlcPtr AlcPtr_set(AlcRes err) {
	#if Alc_SAFE
		ASSERT(err <= AlcRes_max);
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

[[nodiscard]]
Ptr AlcPtr_unwrap_(AlcPtr ptr, ConstPtr func_name, ConstPtr header) {
	AlcRes_assert_(AlcPtr_get(ptr), func_name, header);
	return (Ptr)ptr;
}

void AlcPtr_assert_(AlcPtr ptr, ConstPtr func_name, ConstPtr header) {
	AlcRes_assert_(AlcPtr_get(ptr), func_name, header);
}

#define AlcPtr_UNWRAP(res) \
	AlcPtr_unwrap_(res, PANIC_MSG_HEADER("AlcPtr_UNWRAP(" #res "):"))

#define AlcPtr_ASSERT(res) \
