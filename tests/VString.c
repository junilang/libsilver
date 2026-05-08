int ZZentry(SilverTestContext *ctx) {
	String hello = STRING("こんにちは、世界よ 🌍\n");
	FPRINT(stdout, VString_upcast(hello.data, hello.size, FLAG(VStringFlag, KEEPUTF)) ,"\n");
	return 0;
}
