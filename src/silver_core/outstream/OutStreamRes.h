#define XS \
	X(Ok) \
	X(ErrInternal) \
	X(ErrOverflow) \
	X(ErrInvalidStream) \
	X(ErrUnknown)

typedef enum : u8 {
	#define X(N) OutStreamRes_##N,
		XS
	#undef X
} OutStreamRes;

const String OutStreamRes_repr[] = {
	#define X(N) [OutStreamRes_##N] = String_INIT(#N),
		XS
	#undef X
};

#undef XS

static void OutStreamRes_UNWRAP_internal(OutStreamRes res, ConstPtr func_name, ConstPtr panic_header) {
	if (res) PANIC_internal(func_name, panic_header, OutStreamRes_repr[res].data, nullptr);
}

#define OutStreamRes_UNWRAP(result) \
	OutStreamRes_UNWRAP_internal(result, __func__, PANIC_IDENTIFIER" OutStreamRes_UNWRAP:")
