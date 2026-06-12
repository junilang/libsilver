typedef STRUCTDECL(ArenaAlc_Chunk);

struct ArenaAlc_Chunk {
	ArenaAlc_Chunk *prev;
	ArenaAlc_Units off_head;
	ArenaAlc_Units off_end;
	alignas(ArenaAlc_unit) ubyte data[]; // data aligned to units
};

static_assert(alignof(ArenaAlc_Chunk) == ArenaAlc_unit);

ArenaAlc_Chunk *ArenaAlc_Chunk_allocate(
	const Alc provider, ConstPtr hint, usize least_size, usize size
) {
	AlcAlign req_align = AlcAlign_set(alignof(ArenaAlc_Chunk));
	AlcRelative req_relative = hint ? AlcRelative_Local : AlcRelative_None;


	// ensure space for chunk header and align sizes to unit

	least_size += sizeof(ArenaAlc_Chunk);
	least_size = (least_size + (ArenaAlc_unit - 1)) & (~((usize)ArenaAlc_unit - 1));

	size += sizeof(ArenaAlc_Chunk);
	size = (size + (ArenaAlc_unit - 1)) & (~((usize)ArenaAlc_unit - 1));

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
					goto set_size;
			}
		} else set_size: {
			size = offer_size;
		}

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

	chunk->off_head = 0;
	chunk->off_end = (ArenaAlc_Units)(
		(size - __builtin_offsetof(ArenaAlc_Chunk, data)) / ArenaAlc_unit
	);

	return chunk;
}
