TEST(ZZ) {
	const OutStream log = ctx->log;
	OutStreamRes res;

	#define XPRINT(...) \
		PRINT(&res, log, __VA_ARGS__, "\n"); \
		if (res) goto handle_error;

	XPRINT("Hello ", "World", "!");

	constexpr auto val = 12345;
	constexpr PrintFmt fmt_hex = {
		FIELD(IntFmt_Base, Hex) |
		FLAG(IntFmt_Header)
	};

	XPRINT("dec=",val," hex=",fmt_hex,val);
	XPRINT(u64_max);
	XPRINT(fmt_hex,u64_max);

	constexpr PrintFmt fmt_bin = {
		FIELD(IntFmt_Base, Bin) |
		FIELD_SET(IntFmt_Spacing, 4) |
		FIELD(IntFmt_Delimiter, Dash) |
		FIELD_SET(IntFmt_Digits, 80)
	};

	XPRINT(fmt_bin, u64_max);

	return 0;

	handle_error:;
	return (int)res;

	#undef XPRINT
}
