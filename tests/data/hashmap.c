bool ZZmatch(Ptr cond, Ptr value) {
	return cond == value;
}

usize values[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
};

usize values_delete[] = {
	2, 5, 10, 12, 13, 16
};

int ZZentry(SilverTestContext *ctx) {
	alignas(Ptr) ubyte buffer[8192];

	auto alc_ = StaticAlc_init(buffer, sizeof(buffer));
	auto alc = StaticAlc_upcast(alc_);

	HashMap map = {};

	AlcRes_UNWRAP(HashMap_init(&map, alc, 66));

	PRINTBP(128, Stdout, "power = ",FIELD_GET(HashMap_Power, map.info),"\n");

	// add items
	for (usize i = 0; i < COUNTOF(values); i++) {
		usize *vp = AlcPtr_UNWRAP(
			HashMap_add(&map, u64_hash(hash_base, values[i]), alc)
		);

		*vp = values[i];

		//PRINTBP(128, Stdout, vp, " = ", values[i], "\n");
	}

	// delete some
	for (usize i = 0; i < COUNTOF(values_delete); i++) {
		usize *vp = HashMap_get(
			&map, u64_hash(hash_base, values_delete[i]), &ZZmatch, (Ptr)values_delete[i]
		);

		if (!vp) PANIC("missing item");

		//PRINTBP(128, Stdout, "deleting ", vp, " = ", *vp, "\n");

		HashMap_delete(&map, vp);
	}

	usize map_size = HashMap_size_base << FIELD_GET(HashMap_Power, map.info);
	PRINTBP(128, Stdout, "map_size = ", map_size, "\n");

	// print out hashmap data
	ubyte *table = map.data;
	HashMapItem *items = (Ptr)(table + map_size);
	for (usize i = 0; i < map_size; i++) {
		#if 0
		constexpr PrintFmt hexfmt = {
			FIELD(IntFmt_Base, Hex) |
			FLAG(IntFmt_Header)
		};

		PRINTBP(128, Stdout,
			hexfmt, table[i], " ",
			hexfmt, items[i].hash, " ",
			(usize)items[i].value, " \n"
		);
		#endif
	}

	for (usize i = 0; i < COUNTOF(values); i++) {
		usize *vp = HashMap_get(&map, u64_hash(hash_base, values[i]), &ZZmatch, (Ptr)values[i]);
		if (vp) {
			PRINTBP(128, Stdout, vp, " = ", *vp, "\n");
		}
	}


	return 0;
}
