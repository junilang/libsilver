#define XS \
	X(Ok) \
	X(Partial) \
	X(Locked) \
	X(ErrInvalidReq) \
	X(ErrInvalidSpec) \
	X(ErrInvalidValue) \
	X(ErrOverflow) \
	X(ErrUnsupported) \
	X(ErrUnimplemented) \
	X(ErrUnknown) \
	X(Aux)

	typedef enum : u8 {
		#define X(N) SinkRes_##N,
			XS
		#undef X
	} SinkRes;

	const String SinkRes_str[] = {
		#define X(N) [SinkRes_##N] = String_INIT(#N),
			XS
		#undef X
	};
#undef XS

PANIC_RES_GENERATE(SinkRes, SinkRes_str)

#define SinkRes_ASSERT(res) ASSERT_RES(SinkRes, res)
