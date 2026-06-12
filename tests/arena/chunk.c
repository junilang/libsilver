int ZZentry(SilverTestContext *ctx) {
	auto chunk = ArenaAlc_Chunk_allocate(OsAlc, nullptr, 0, 4096);

	{
		AlcRes res = AlcRes_get(chunk);
		switch (res) {
			default:
				PRINTB(128, Stdout, "allocation error: ",res,"\n");
				return 1;
			case AlcRes_Ok:
		}
	}

	PRINTB(128, Stdout,
		"chunk=",chunk,
		" off_head=",chunk->off_head,
		" off_end=",chunk->off_end,
		"\n"
	);

	return SilverTest_OK;
}
