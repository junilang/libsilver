int ZZentry(SilverTestContext *ctx) {
	constexpr usize size_base = 8;

	constexpr u8 sentinel_empty = 0b10000000;
	constexpr u8 sentinel_vacated = 0b11111111;
	constexpr u8 hash_mask = 0b01111111;

	u64 hash = memhash(hash_base, STR("Hello World!"));
	hash = 0;

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

	constexpr u64 spread_const = 0x0101010101010101ull;

	u64 spread = spread_const * target;

	for (uint i = 0; i < 2; i++) {
		auto tbl = ((u64*)lookup)[i];
		auto xor_res = tbl ^ spread;
		auto res = (xor_res - spread_const) & (~xor_res) & 0x8080808080808080ull;

		PRINTBP(512, Stdout, fmt,tbl,"\n",fmt,xor_res,"\n",fmt,res,"\n\n");

		PRINTBP(128, Stdout, __builtin_ctzg(res),"\n");
	}

	return 0;
}
