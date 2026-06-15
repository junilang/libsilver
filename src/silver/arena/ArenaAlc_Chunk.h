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
	alignas(ArenaAlc_unit) ubyte data[];
};

static_assert(alignof(ArenaAlc_Chunk) == ArenaAlc_unit);

// bytes per bitmap byte
constexpr uint ArenaAlc_bitfactor = ArenaAlc_unit * ubyte_width;

// bitmap adjusted size
usize ArenaAlc_Chunk_bmasize(usize size) {
	size = usize_align(size, ArenaAlc_unit);
	size +=
		// number of bitmap bytes (unit aligned) to cover size of bytes
		usize_align((size + (ArenaAlc_bitfactor - 1)) / ArenaAlc_bitfactor, ArenaAlc_unit)
		// chunk header - already aligned
		+ sizeof(ArenaAlc_Chunk)
	;

	return size;
}

ArenaAlc_Chunk *ArenaAlc_Chunk_allocate(
	const Alc provider, ConstPtr hint, usize least_size, usize size
) {
	AlcAlign req_align = AlcAlign_set(ArenaAlc_unit);
	AlcRelative req_relative = hint ? AlcRelative_Local : AlcRelative_None;

	least_size = ArenaAlc_Chunk_bmasize(least_size);
	size = ArenaAlc_Chunk_bmasize(size);

	AlcNrs offer;
	if (size <= least_size) { // negotiate using least intent
		AlcNrq request =
			FIELD_SETN(AlcSize, least_size) |
			FIELD_SETN(AlcAlign, req_align) |
			FIELD_SETN(AlcRelative, req_relative) |
			FIELD_SET(AlcNrq_Intent, Least) |
			FLAG(AlcReq, Zero)
		;

		offer = Alc_negotiate(provider, request, hint, nullptr);
		switch (FIELD_GET_CAST(AlcNrs_Offer, offer)) {
			default: goto negotiate_failed;
			case AlcNrs_Offer_Accept:
		}

		size = FIELD_GET(AlcSize, offer);

	} else { // negotiate loosely around requested size
		usize alt_offers[2];

		AlcNrq request =
			FIELD_SETN(AlcSize, size) |
			FIELD_SETN(AlcAlign, req_align) |
			FIELD_SETN(AlcRelative, req_relative) |
			FIELD_SETN(AlcNrq_Alts, 2) |
			FIELD_SET(AlcNrq_Intent, Loose) |
			FLAG(AlcReq, Zero)
		;

		offer = Alc_negotiate(provider, request, hint, alt_offers);
		switch (FIELD_GET_CAST(AlcNrs_Offer, offer)) {
			default: goto negotiate_failed;
			case AlcNrs_Offer_Accept:
		}

		usize offer_size = FIELD_GET(AlcSize, offer);
		if (offer_size < least_size) {
			const u8 alts = FIELD_GET(AlcNrs_Alts, offer);
			for (u8 i = 0; i < alts; i++) {
				offer_size = alt_offers[i];
				if (offer_size >= least_size)
					size = offer_size;
			}
		} else
			size = offer_size;


		// no agreeable size found, try to allocate
		// with the original requested size anyway
	}

	if (0) negotiate_failed: {
		AlcRes reason = FIELD_GET(AlcNrs_RefuseReason, offer);
		if (!reason) reason = AlcRes_ErrUnknown;
		return AlcRes_set(reason);
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

	if (least_size + ArenaAlc_unit > size) {
		size = least_size;
	}

	size -= sizeof(ArenaAlc_Chunk);

	// compute bitmap size (unit aligned)
	usize bitmap_size = usize_align(
		(size + ArenaAlc_bitfactor) / (ArenaAlc_bitfactor + 1), ArenaAlc_unit
	);

	auto bitmap_units = (ArenaAlc_Units)(bitmap_size / ArenaAlc_unit);
	auto data_units = (ArenaAlc_Units)((size - bitmap_size) / ArenaAlc_unit);

	// already zeroed
	auto bitmap = (ArenaAlc_Unit*)chunk->data;

	// number of units the bitmap covers that are past the end of the allocated buffer
	usize lost_units = bitmap_units * ArenaAlc_bitfactor - data_units;

	// set upper bits on last bitmap unit to account for inaccessible bytes
	bitmap[bitmap_units - 1] = ~((ArenaAlc_Unit)(~0ull) >> lost_units);

	chunk->bitmap_units = bitmap_units;
	chunk->off_end = 0;

	return chunk;
}
