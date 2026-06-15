const PrintFmt ZZfmt = {
	FIELD_SETN(IntFmt_Digits, 64) | FIELD_SET(IntFmt_Base, Bin) |
	FIELD_SETN(IntFmt_Spacing, 8)
};

constexpr u32 ZZunit = 8;

void ZZfind_run(u64 *bits, u32 bits_count, usize base, ualign align) {
	u64 val = *(bits++);
	u32 run = 0;
	u32 run_idx = 0;
	u32 i = 0;

	while (i < bits_count) {
		if (val == 0) {
			u32 d = 64 - (i % 64);
			i += d;
			run += d;
			if (i < bits_count) {
				val = *(bits++);
				continue;
			} else {
				break;
			}
		}

		u32 tz = (u32)__builtin_ctzg(val);
		run += tz;

		PRINTB(128, Stdout, "run: ",run," at ",run_idx,"\n");

		val = ~(val >> tz);
		u32 tz2 = (u32)__builtin_ctzg(val);
		val = (~val) >> tz2;


		i += tz + tz2;
		run = 0;
		run_idx = i;
	}

	PRINTB(128, Stdout, "run: ",run," at ",run_idx,"\n");
}

int ZZentry(SilverTestContext *ctx) {
	u64 bits[2] = {1ull << 12, 1ull << 32};


	PRINTB(256, Stdout, ZZfmt,bits[1]," ",ZZfmt,bits[0],"\n");
	ZZfind_run(bits, sizeof(bits) * 8, usize_align(32132124511, ZZunit), 256);


	return SilverTest_OK;
}
