int ZZentry(SilverTestContext *ctx) {
	PrintFmt fmt = {
		FIELD_SET(IntFmt_Base, Oct) | FLAG(IntFmt, Header, Capitalize) |
		FIELD_SETN(IntFmt_Spacing, 4) | FIELD_SET(IntFmt_Delimiter, Underscore)
	};


	PRINTB(128, Stdout, fmt, u64_max,"\n");
	PRINTB(128, Stdout, &fmt,"\n");


	return 0;
}
