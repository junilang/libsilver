ENUM_GENERATE(FsRes, u8,
	Ok,
	ErrInvalidPath,
	ErrPathOverflow,
	ErrInvalidFileMode,
	ErrInvalidMappingMode,
	ErrPermission,
	ErrNotFound,
	ErrUnknown,
	Aux
)

PANIC_RES_GENERATE(FsRes, FsRes_str)

#define FsRes_ASSERT(result) ASSERT_RES(FsRes, result)
