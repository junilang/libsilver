int ZZentry(SilverTestContext *ctx) {
	ArenaAlc arena;
	auto res = ArenaAlc_init(&arena, OsAlc, 4096);
	switch (res) {
		default:
			PRINTB(128, Stdout, "arena init failed: ",res,"\n");
			return 1;
		case AlcRes_Ok:
	}

	Ptr buf = ArenaAlc_new(&arena,
		FIELD_SETN(AlcSize, sizeof(int)) | FIELD_SETN(AlcAlign, AlcAlign_set(alignof(int))),
	 	nullptr
	);
	{
		auto res = AlcRes_get(buf);
		switch (res) {
			default:
				PRINTB(128, Stdout, "allocation failed: ",res,"\n");
				return 1;
			case AlcRes_Ok:
		}
	}

	PRINTB(128, Stdout,
		"buf=",buf,
		"\n"
	);

	return SilverTest_OK;
}
