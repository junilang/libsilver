int ZZentry(SilverTestContext *ctx) {
	PrintFmt fmt = {
		FIELD_SET(IntFmt_Base, Bin) | FLAG(IntFmt, Header, Capitalize)
	};


	PRINTB(128, Stdout, fmt, 255,"\n");
	PRINTB(128, Stdout, &fmt,"\n");


	return 0;
}
