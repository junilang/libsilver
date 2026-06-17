int ZZentry(SilverTestContext *ctx) {
	usize offers[4];

	AlcReq request = {
		.intent = AlcIntent_Query,
		.range = AlcRange_Loose,
		.size = 4096,
		.align = 8,
		.offers_size = 4
	};

	auto res = Alc_invoke(OsAlc, &request, offers, nullptr);

	switch (AlcRes_get(res)) {
		default: {
			PRINTB(128, Stdout, "offer refused :(\n");
			return 1;
		}

		case AlcRes_Ok:;
	}

	for (u8 i = 0; i < request.offers_size; i++) {
		PRINTB(128, Stdout, "offers[",i,"]: ",offers[i],"\n");
	}

	return 0;
}
