int ZZentry(SilverTestContext *ctx) {
	VString x = {
		String_INIT("Hello")
	};

	PRINTP(Stdout, (PrintFmt){
		FLAGS(VStringFmt, Code)
	}, x);

	return 0;
}
