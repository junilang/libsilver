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

[[noreturn, gnu::noinline]]
void OutStreamRes_panic_(
	OutStreamRes res,
	ConstPtr func_name,
	ConstPtr header
) {
	#define XMSG "Aux("
	constexpr auto msg_len = __builtin_strlen(XMSG);
	constexpr auto buf_size = 16;
	ubyte buf[msg_len + buf_size + 4] = {XMSG};

	String msg[3] = {
		String_INIT(func_name),
		String_INIT(header)
	};

	if (res >= OutStreamRes_Aux) {
		auto const r = ullong_tostr(
			res, FLAG(IntFmt_ZeroOffset), buf + msg_len, buf_size
		);
		if (r < 0) msg[2] = (String)String_INIT("Aux(?tostr_error?)");
		else {
			auto const l = (usize)r;
			buf[msg_len + l] = ')';
			msg[2] = (String){.data = buf, .size = msg_len + l + 1 };
		}
	} else {
		msg[2] = OutStreamRes_repr[res];
	}

	panic_msg_(msg, COUNTOF(msg));
}

void OutStreamRes_assert_(
	OutStreamRes res,
	ConstPtr func_name,
	ConstPtr header
) {
	if (res) OutStreamRes_panic_(res, func_name, header);
}

#define OutStreamRes_ASSERT(result) \
	OutStreamRes_assert_(result, __func__, SOURCE_IDENTIFIER " OutStreamRes_ASSERT(" #result "):")
