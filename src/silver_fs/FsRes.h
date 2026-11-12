#define XS \
	X(Ok) \
	X(ErrInvalidPath) \
	X(ErrPathOverflow) \
	X(ErrInvalidFileMode) \
	X(ErrInvalidMappingMode) \
	X(ErrPermission) \
	X(ErrNotFound) \
	X(ErrUnknown) \
	X(Aux)

typedef enum : u8 {
	#define X(N) FsRes_##N,
		XS
	#undef X
} FsRes;

const String FsRes_str[] = {
	#define X(N) [FsRes_##N] = String_INIT("FsRes_"#N),
		XS
	#undef X
};

#undef XS

PANIC_RES_GENERATE(FsRes, FsRes_str)

#define FsRes_ASSERT(result) ASSERT_RES(FsRes, result)
