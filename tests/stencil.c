int ZZentry(SilverTestContext *ctx) {
	constexpr usize size_base = 8;

	constexpr u8 sentinel_empty = 0b10000000;
	constexpr u8 sentinel_vacated = 0b11111111;
	constexpr u8 hash_mask = 0b01111111;

	u64 hash = memhash(hash_base, STR("Hello World!"));
	//hash = 0;

	u8 target = hash & hash_mask;

	constexpr PrintFmt fmt = {
		FIELD(IntFmt_Base, Bin) |
		FIELD_SET(IntFmt_Spacing, 8) |
		FIELD_SET(IntFmt_Digits, 64) |
		FLAG(IntFmt_FillSpacing)
	};

	alignas(usize) u8 lookup[16] = {};

	for (usize i = 0; i < sizeof(lookup); i++) {
		lookup[i] = sentinel_empty;
	}

	lookup[10] = target;
	lookup[11] = sentinel_vacated;
	lookup[5] = u32_hash(hash_base, 321321) & hash_mask;

	constexpr u64 spread_const = 0x0101010101010101ull;

	u64 spread = spread_const * target;

	spread |= usize_max << (8 * 4);

	PRINTBP(128, Stdout, fmt, spread, "\n\n");

	for (uint i = 0; i < 2; i++) {
		auto tbl = ((u64*)lookup)[i];
		auto xor_res = tbl ^ (spread_const * sentinel_empty);
		auto res = (xor_res - spread_const) & (~xor_res) & 0x8080808080808080ull;


		auto empty = spread_const * sentinel_empty;

		auto s1 = tbl & empty;
		auto s2 = tbl & (~empty);

		//PRINTBP(512, Stdout, fmt, tbl, "\n", fmt, s1, "\n", fmt, s2, "\n\n");


		PRINTBP(512, Stdout, fmt,tbl,"\n",fmt,xor_res,"\n",fmt,res,"\n\n");
	}

	return 0;
}
