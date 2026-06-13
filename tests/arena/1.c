int ZZentry(SilverTestContext *ctx) {
	ArenaAlc arena;
	auto res = ArenaAlc_init(&arena, OsAlc, 4096);
	switch (res) {
		default:
			PRINTB(128, Stdout, "arena init failed: ",res,"\n");
			return 1;
		case AlcRes_Ok:
	}

	for (uint i = 0; i < 10; i++) {
		Ptr buf = ArenaAlc_new(&arena,
			FIELD_SETN(AlcSize, sizeof(int) * 100) | FIELD_SET(AlcAlign, 256B),
		 	nullptr
		);
		auto res = AlcRes_get(buf);
		switch (res) {
			default:
				PRINTB(128, Stdout, "allocation failed: ",AlcRes_repr[res],"\n");
				return 1;
			case AlcRes_Ok:
		}
		PRINTB(128, Stdout,
			i," buf=",buf,
			"\n"
		);
	}

	return SilverTest_OK;
}
