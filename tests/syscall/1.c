int ZZentry(SilverTestContext *ctx) {
	FPRINT_BUFFERED(128, Stdout, "hello world ", -80005, "\n");

	return SilverTest_OK;
}
