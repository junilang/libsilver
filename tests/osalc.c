int ZZentry(SilverTestContext *ctx) {
	AlcOffer offers[4] = {};

	AlcReq req = FIELD_SET(AlcIntent, Loose) |
		FIELD_SETN(AlcOffersSize, 4) |
		FIELD_SET(AlcAlign, 128B) |
		FIELD_SETN(AlcSize, 8072);

	PRINTB(128, Stdout,
		"req=", ((PrintFmt){
			FIELD_SET(IntFmt_Base, Bin) |
			FIELD_SETN(IntFmt_Spacing, 4) |
			FIELD_SETN(IntFmt_Digits, 64)
		}), req,
		"\nintent=", FIELD_GET(AlcIntent, req),
		" offers_size=", FIELD_GET(AlcOffersSize, req),
		" align=", FIELD_GET(AlcAlign, req),
		" size=", FIELD_GET(AlcSize, req),
		"\n"
	);

	AlcRes res = Alc_query(OsAlc, req, nullptr, offers);

	switch (res) {
		default: {
			PRINTB(128, Stdout, "offer refused :(\n");
			return 1;
		}

		case AlcRes_Ok:;
	}

	usize offer = FIELD_GET(AlcSize, res);
	PRINTB(128, Stdout, "offer: ",offer,"\n");

	for (AlcOffersSize i = 0; i < 4; i++) {
		PRINTB(128, Stdout, "alt[",i,"]: ",offers[i],"\n");
	}

	Ptr mem = Alc_new(OsAlc,
		FIELD_SETN(AlcSize, 512) | FIELD_SET(AlcAlign, 128B),
		nullptr
	);

	switch (AlcRes_get(mem)) {
		default:
			PRINTB(128, Stdout, "allocation failed\n");
			return 1;
		case AlcRes_Ok:;
	}

	PRINTB(128, Stdout, "allocated: ",mem,"\n");

	switch (Alc_delete(OsAlc, mem, 0)) {
		default:
			PRINTB(128, Stdout, "failed to delete\n");
			return 1;
		case AlcRes_Ok:;
	}

	return 0;
}
