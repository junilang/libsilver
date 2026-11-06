#define XS \
	X(Ok) \
	X(ErrInternal) \
	X(ErrInvalidStream) \
	X(ErrInvalidInput) \
	X(ErrOverflow) \
	X(ErrUnknown) \
	X(Aux)

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

PANIC_RES_GENERATE(OutStreamRes)

#define OutStreamRes_ASSERT(result) ASSERT_RES(OutStreamRes, result)
