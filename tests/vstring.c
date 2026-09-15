int ZZentry(SilverTestContext *ctx) {
	VString x = {
		String_INIT("Hello 🤪\n")
	};

	PRINTP(Stdout, (PrintFmt){
		FLAGS(VStringFmt, NoUTF)
	}, x);

	return 0;
}
