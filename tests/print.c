OutStreamRes ZZprintable_test(Ptr this, PrintFmt fmt, OutStream os) {
	if (fmt.value) {
		return OutStreamRes_ErrUnknown;
	}

	return OutStream_write(os, USTR("Hello World!"));
}

int ZZentry(SilverTestContext *ctx) {
	StaticPrintable repr = {
		.print = &ZZprintable_test, .this = nullptr
	};

	return SilverTest_Ok;
}
