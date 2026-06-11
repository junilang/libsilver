int ZZentry() {
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
