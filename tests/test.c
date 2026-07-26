int ZZentry(SilverTestContext *ctx) {
	constexpr PrintFmt fmt = {
		FIELD(IntFmt_Base, Bin) |
		FIELD_SET(IntFmt_Digits, ubyte_width)
	};

	u8 x = 0;
	x -= 1;

	PRINTP(Stdout, fmt, x,"\n");

	return 0;
}
