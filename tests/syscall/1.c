int ZZentry(SilverTestContext *ctx) {
	auto result = linux_write(linux_stdout, USTR("hello world!\n"));

	if (result < -1) {
		linux_exit((int)(-result));
	}


	return SilverTest_ERROR;
}
