int ZZentry(SilverTestContext *ctx) {
	alignas(Ptr) ubyte buffer[2048];

	auto alc_ = StaticAlc_init(buffer, 2048);
	auto alc = StaticAlc_upcast(alc_);

	Vec v = {};

	for (uint i = 0; i < 128; i++) {
		uint *elem = AlcPtr_UNWRAP(Vec_push(&v, alc, sizeof(uint)));

		*elem = i;
	}

	int *elems = Vec_begin(&v);
	usize count = Vec_count(&v, sizeof(int));
	for (usize i = 0; i < count; i++) {
		PRINTBP(128, Stdout, "[",i,"] = ",elems[i],"\n");
	}

	return SilverTest_Ok;
}
