int ZZentry(SilverTestContext *ctx) {
	auto os = FileOutStream_upcast(linux_stdout);

	PRINT_BUFFERED(128, os, "hello world ", -80005, "\n");

	return SilverTest_OK;
}
