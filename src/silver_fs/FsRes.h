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

static void FsRes_UNWRAP_internal(FsRes res, ConstPtr func_name, ConstPtr panic_header) {
	if (res) PANIC_internal(func_name, panic_header, FsRes_repr[res].data, nullptr);
}

#define FsRes_UNWRAP(result) \
	FsRes_UNWRAP_internal(result, __func__, PANIC_IDENTIFIER" FsRes_UNWRAP:")
