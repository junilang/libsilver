bool ZZmatch(Ptr cond, Ptr value) {
	return cond == value;
}

usize values[] = {1,2,3,4,5,6,7,8,9,10};
usize values_delete[] = {1,2,3,4};

int ZZentry(SilverTestContext *ctx) {
	alignas(Ptr) ubyte buffer[8192];

	BumpAlc bumpalc;
	BumpAlc_init(&bumpalc, buffer, sizeof(buffer));
	auto alc = BumpAlc_asAlc(&bumpalc);

	HashMap map = {};

	AlcRes_ASSERT(HashMap_init(&map, alc, 66));

	PRINTA(Stdout, "power = ",FIELD_GET(HashMap_Power, map.info),"\n");

	// add items
	for (usize i = 0; i < _Countof(values); i++) {
		usize *vp = AlcPtr_UNWRAP(
			HashMap_add(&map, alc, u64_crc64(crc64_base, values[i]))
		);

		*vp = values[i];

		//PRINTBP(128, Stdout, vp, " = ", values[i], "\n");
	}

	// delete some
	for (usize i = 0; i < _Countof(values_delete); i++) {
		usize *vp = HashMap_get(
			&map, u64_crc64(crc64_base, values_delete[i]), &ZZmatch, (Ptr)values_delete[i]
		);

		ASSERT(vp, "missing item");

		//PRINTBP(128, Stdout, "deleting ", vp, " = ", *vp, "\n");

		HashMap_delete(&map, vp);
	}

	usize map_size = HashMap_basesize << FIELD_GET(HashMap_Power, map.info);
	PRINTA(Stdout, "map_size = ", map_size, "\n");

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

	for (usize i = 0; i < _Countof(values); i++) {
		usize *vp = HashMap_get(&map, u64_crc64(crc64_base, values[i]), &ZZmatch, (Ptr)values[i]);
		if (vp) {
			PRINTA(Stdout, vp, " = ", *vp, "\n");
		}
	}


	return 0;
}
