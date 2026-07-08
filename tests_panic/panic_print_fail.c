OutStreamRes ZZprintable_fail(Ptr this, PrintFmt fmt, OutStream os) {
	return OutStreamRes_ErrUnknown;
}

int ZZentry(SilverTestContext *ctx) {
	StaticPrintable p = {
		.print = &ZZprintable_fail,
		.this = nullptr
	};

	PANIC_PRINT(p);


	return 666;
}
