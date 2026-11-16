ENUM_GENERATE(SinkRes, u8,
	Ok,
	Partial,
	Locked,
	ErrInvalidReq,
	ErrInvalidAux,
	ErrInvalidSpec,
	ErrInvalidValue,
	ErrOverflow,
	ErrUnsupported,
	ErrUnimplemented,
	ErrUnknown,
	Aux,
)

PANIC_RES_GENERATE(SinkRes, SinkRes_str)

#define SinkRes_ASSERT(res) ASSERT_RES(SinkRes, res)
