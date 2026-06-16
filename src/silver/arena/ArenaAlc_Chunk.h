typedef STRUCTDECL(ArenaAlc_Chunk);

struct ArenaAlc_Chunk {
	ArenaAlc_Chunk *prev;
	ArenaAlc_Chunk *next;
	ArenaAlc_Units prev_maxfree;
	ArenaAlc_Units maxfree;
	union {
		ArenaAlc_Units bitmap_units;
		ArenaAlc_Units off_head;
	};
	ArenaAlc_Units off_end;
	alignas(ArenaAlc_Unit_size) ubyte data[];
};

static_assert(alignof(ArenaAlc_Chunk) == ArenaAlc_Unit_size);

// bitmap adjusted size
usize ArenaAlc_Chunk_bmasize(/* size of usable memory in bytes */ usize size) {
	// align size to units
	size = usize_align(size, ArenaAlc_Unit_size);

	// number of bytes covered by unit of bitmap
	constexpr uint bitfactor = ArenaAlc_Unit_size * ArenaAlc_Unit_width;

	// calculate units of bitmap needed to cover usable memory
	usize bitmap_units = (size + (bitfactor - 1)) / bitfactor;

	size += bitmap_units * ArenaAlc_Unit_size;

	// add size of header (already aligned to units)
	size += sizeof(ArenaAlc_Chunk);

	return size;
}

ArenaAlc_Chunk *ArenaAlc_Chunk_allocate_bm(
	const Alc provider, ConstPtr hint, usize least_usable_size, usize size
) {
	AlcAlign req_align = AlcAlign_set(ArenaAlc_Unit_size);
	AlcRelative req_relative = hint ? AlcRelative_Local : AlcRelative_None;

	usize least_size = ArenaAlc_Chunk_bmasize(least_usable_size);
	size = ArenaAlc_Chunk_bmasize(size);

	if (size <= least_size) { // negotiate using least intent
		size = least_size;

		AlcReq request =
			FIELD_SETN(AlcSize, size) |
			FIELD_SETN(AlcAlign, req_align) |
			FIELD_SETN(AlcRelative, req_relative) |
			FIELD_SET(AlcIntent, Least) |
			FIELD_SETN(AlcOffersSize, 1) |
			FLAG(AlcReq, Zero)
		;

		AlcOffer offer = 0;
		auto res = Alc_query(provider, request, hint, &offer);
		switch (res) {
			default: return AlcRes_set(res);
			case AlcRes_Ok:
		}

		if (offer)
			size = FIELD_GET(AlcSize, offer);

	} else { // negotiate loosely around requested size
		AlcReq request =
			FIELD_SETN(AlcSize, size) |
			FIELD_SETN(AlcAlign, req_align) |
			FIELD_SETN(AlcRelative, req_relative) |
			FIELD_SET(AlcIntent, Loose) |
			FIELD_SETN(AlcOffersSize, 2) |
			FLAG(AlcReq, Zero)
		;

		AlcOffer offers[2] = {};
		auto res = Alc_query(provider, request, hint, offers);
		switch (res) {
			default: return AlcRes_set(res);
			case AlcRes_Ok:
		}

		for (uint i = 0; i < 2; i++) {
			if (!offers[i]) break;
			usize offer_size = FIELD_GET(AlcSize, offers[i]);
			if (offer_size >= least_size) {
				size = offer_size;
				break;
			}
		}
	}

	AlcReq request =
		FIELD_SETN(AlcSize, size) |
		FIELD_SETN(AlcAlign, req_align) |
		FIELD_SETN(AlcRelative, req_relative) |
		FLAG(AlcReq, Zero)
	;

	ArenaAlc_Chunk *chunk = Alc_new(provider, request, hint);
	if (AlcRes_get(chunk)) // propagate error
		return chunk;

	size -= sizeof(ArenaAlc_Chunk);

	auto units = (ArenaAlc_Units)(size / ArenaAlc_Unit_size);

	// example: when unit width = 64, we need a 1 unit bitmap for 64 memory units,
	// 	in total 65, so to get number of bitmap units from total size, we divide
	// 	the total number of units by unit_width + 1 (ceil)
	ArenaAlc_Units bitmap_units = (units + ArenaAlc_Unit_width) / (ArenaAlc_Unit_width + 1);

	chunk->bitmap_units = bitmap_units;
	chunk->off_end = 0;

	auto bitmap = (ArenaAlc_Unit*)chunk->data;

	ArenaAlc_Units rest_units = units - bitmap_units;
	ArenaAlc_Units usable_units = bitmap_units * ArenaAlc_Unit_width;
	if (usable_units > rest_units) {
		// set last bits of bitmap to 1 to signify unusable memory
		bitmap[bitmap_units - 1] = (ArenaAlc_Unit)(~0ull) >> (usable_units - rest_units);
	}

	return chunk;
}
