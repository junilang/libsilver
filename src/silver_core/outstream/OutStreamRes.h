#define XS \
	X(Ok) \
	X(ErrInternal) \
	X(ErrOverflow) \
	X(ErrUnknown) \
	X(ErrInvalidStream)

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
