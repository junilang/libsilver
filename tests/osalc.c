int ZZentry(SilverTestContext *ctx) {
	usize res_alts[4];

	AlcNrq req = FIELD_SET(AlcNrq_Intent, Loose) |
		FIELD_SETN(AlcNrq_Alts, 4) |
		FIELD_SET(AlcAlign, 128B) |
		FIELD_SETN(AlcSize, 8072);

	PRINTB(128, Stdout,
		"req=", ((PrintFmt){
			FIELD_SET(IntFmt_Base, Bin) |
			FIELD_SETN(IntFmt_Spacing, 4) |
			FIELD_SETN(IntFmt_Digits, 64)
		}), req,
		"\nintent=", FIELD_GET(AlcNrq_Intent, req),
		" alts=", FIELD_GET(AlcNrq_Alts, req),
		" align=", FIELD_GET(AlcAlign, req),
		" size=", FIELD_GET(AlcSize, req),
		"\n"
	);

	AlcNrs res = Alc_negotiate(OsAlc, req, nullptr, res_alts);

	switch (FIELD_GET_CAST(AlcNrs_Offer, res)) {
		default: {
			PRINTB(128, Stdout, "offer refused :(\n");
			return 1;
		}

		case AlcNrs_Offer_Accept:;
	}

	usize offer = FIELD_GET(AlcSize, res);
	PRINTB(128, Stdout, "offer: ",offer,"\n");

	u8 alts_size = FIELD_GET(AlcNrs_Alts, res);
	for (u8 i = 0; i < alts_size; i++) {
		PRINTB(128, Stdout, "alt[",i,"]: ",res_alts[i],"\n");
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
