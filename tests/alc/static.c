int ZZentry(SilverTestContext *ctx) {
	AlcReq req = {
		.intent = AlcIntent_New,
		.size = 12000,
		.align = StaticAlc_minalign,
	};

	Ptr mem = Alc_invoke(OsAlc, &req, nullptr, nullptr);
	switch (AlcRes_get(mem)) {
		default: PANIC();
		case AlcRes_Ok:
	}

	PRINTBP(128, Stdout, "Allocated ",req.size,"\n");

	auto const static_alc = StaticAlc_init(mem, req.size);
	auto const alc = StaticAlc_upcast(static_alc);

	for (uint i = 1; i < 50; i++) {
		Ptr mem = Alc_new(alc, 1024);
		if (AlcRes_get(mem)) {
			PRINTBP(128, Stdout, i, " -> ",AlcRes_repr[AlcRes_get(mem)],"\n");
			break;
		}

		mem = Alc_resize(alc, mem, 2048);
		if (AlcRes_get(mem)) {
			PRINTBP(128, Stdout, i, " -> ", AlcRes_repr[AlcRes_get(mem)],"\n");
			break;
		}

		PRINTBP(128, Stdout, i," -> ",mem,"\n");
	}

	switch (Alc_delete(OsAlc, mem)) {
		default: PANIC();
		case AlcRes_Ok:
	}

	return SilverTest_Ok;
}
