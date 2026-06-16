void ZZcalc(usize least_usable_size, usize size) {
	size -= sizeof(ArenaAlc_Chunk);

	auto units = (ArenaAlc_Units)(size / ArenaAlc_Unit_size);

	// example: when unit width = 64, we need a 1 unit bitmap for 64 memory units,
	// 	in total 65, so to get number of bitmap units from total size, we divide
	// 	the total number of units by unit_width + 1 (ceil)
	ArenaAlc_Units bitmap_units = (units + ArenaAlc_Unit_width) / (ArenaAlc_Unit_width + 1);

	ArenaAlc_Units rest_units = units - bitmap_units;

	/*
	PRINTB(128, Stdout,
		"lsize=",least_usable_size,
		" size=",size,
		" units=",units,
		" bitmap_units=",bitmap_units,
		" rest_units=",rest_units,
		"\n"
	);
	*/

	if (rest_units * ArenaAlc_Unit_size < least_usable_size) {
		bitmap_units--;
		rest_units++;
		PRINTB(128, Stdout,
			"OVERSHOOT!\n"
		);
	}

	// one unit per bitmap bit
	ArenaAlc_Units usable_units = bitmap_units * ArenaAlc_Unit_width;
	if (usable_units > rest_units) {
		usable_units = rest_units;
	}
}

int ZZentry(SilverTestContext *ctx) {

	for (usize s = 0; s < 100000000; s += 1) {
		ZZcalc(s, ArenaAlc_Chunk_bmasize(s));
	}


	return SilverTest_OK;
}
