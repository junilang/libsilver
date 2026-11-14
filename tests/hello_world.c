int ZZentry(SilverTestContext *ctx) {
	PRINTA(Stdout, "Hello World!\n");
	PRINTA(Stdout, S(FIELD(IntFmt_Base, Bin)), 123, "\n");


	return 0;
}
