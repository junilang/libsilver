OutStreamRes ZZprintable_fail(Ptr this, PrintFmt fmt, OutStream os) {
	return OutStreamRes_ErrUnknown;
}

TEST(ZZ) {
	StaticPrintable p = {
		.print = &ZZprintable_fail,
		.this = nullptr
	};

	PRINTA(Stdout, p);
	return 666;
}
