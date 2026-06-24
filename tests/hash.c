int ZZentry(SilverTestContext *ctx) {
	PRINTB(128, Stdout,
		((PrintFmt){FIELD_SET(IntFmt_Base, Hex) | FLAG(IntFmt, Header)}),
		u64_fnv1a((usize)&ctx, fnv1a_base),
		"\n"
	);

	Stdout;

	return SilverTest_OK;
}
