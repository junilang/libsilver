int ZZentry(SilverTestContext *ctx) {
	PRINTB(128, Stdout,
		((PrintFmt){FIELD_SET(IntFmt_Base, Hex) | FLAG(IntFmt, Header)}),
		u64_fnv1a((usize)&ctx, hash_base),
		"\n"
	);

	return SilverTest_OK;
}
