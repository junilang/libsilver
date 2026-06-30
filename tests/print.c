OutStreamRes ZZprintable_test(Ptr this, PrintFmt fmt, OutStream os) {
	if (fmt.value) {
		return OutStreamRes_ErrUnknown;
	}

	return OutStream_write(os, STR("Hello World!"));
}

int ZZentry(SilverTestContext *ctx) {
	StaticPrintable repr = {
		.print = &ZZprintable_test, .this = nullptr
	};

	PRINTP(Stdout, "Hewwo Wowld :3\n");

	return SilverTest_Ok;
}
