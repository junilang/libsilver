#define XS \
	X(Ok) \
	X(ErrInvalidPath) \
	X(ErrPathOverflow) \
	X(ErrInvalidFileMode) \
	X(ErrInvalidMappingMode) \
	X(ErrPermission) \
	X(ErrNotFound) \
	X(ErrUnknown)

typedef enum : u8 {
	#define X(N) FsRes_##N,
		XS
	#undef X
} FsRes;

const String FsRes_repr[] = {
	#define X(N) [FsRes_##N] = String_INIT("FsRes_"#N),
		XS
	#undef X
};

#undef XS

[[noreturn, gnu::noinline]]
void FsRes_panic_(FsRes res, ConstPtr func_name, ConstPtr header) {
	// TODO print res value
	String msg[] = {
		String_INIT(func_name),
		String_INIT(header)
	};
	panic_msg_(msg, COUNTOF(msg));
}

void FsRes_assert_(FsRes res, ConstPtr func_name, ConstPtr header) {
	if (res) FsRes_panic_(res, func_name, header);
}

#define FsRes_ASSERT(result) \
	FsRes_assert_(result, __func__, SOURCE_IDENTIFIER " FsRes_ASSERT(" #result "):")
