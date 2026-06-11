int ZZentry() {
	PrintFmt fmt = {
		//FIELD_SET(IntFmt_Base, Hex) |
		FIELD_SETN(IntFmt_Spacing, 4) |
		FIELD_SETN(IntFmt_Digits, 16)
	};

	for (int i = -5; i < 5; i++) {
		PRINTB(128, Stdout, fmt, (uint)i, "\n");
	}

	return 0;
}
