typedef STRUCTDECL(ArenaAlc_Chunk);

typedef enum : u8 {
	ArenaAlc_ChunkType_Bitmapped,
} ArenaAlc_ChunkType;

struct ArenaAlc_Chunk {
	ArenaAlc_Chunk *prev;
	ArenaAlc_Chunk *next;
	ArenaAlc_Units prev_maxfree;
	ArenaAlc_Units maxfree;
	union {
		ArenaAlc_Units bitmap_units;
	};
	ArenaAlc_ChunkType type;
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

// allocate bitmapped chunk
ArenaAlc_Chunk *ArenaAlc_Chunk_bmallocate(
	Alc provider, ConstPtr hint, usize least_usable_size, usize size
) {
	AlcRelative req_relative = hint ? AlcRelative_Local : AlcRelative_None;

	usize least_size = ArenaAlc_Chunk_bmasize(least_usable_size);
	size = ArenaAlc_Chunk_bmasize(size);

	if (size <= least_size) { // negotiate using least range
		size = least_size;

		usize offer;
		AlcReq request = {
			.intent = AlcIntent_Query,
			.size = size,
			.align = ArenaAlc_Unit_size,
			.range = AlcRange_Least,
			.relative = req_relative,
			.hint = hint,
			.flags = FLAG(AlcFlag, Zero),
			.offers_size = 1
		};

		auto res = Alc_invoke(provider, &request, &offer, nullptr);
		switch (AlcRes_get(res)) {
			default: return res;
			case AlcRes_Ok:
		}

		if (request.offers_size)
			size = offer;


	} else { // negotiate loosely around requested size
		usize offers[2];
		AlcReq request = {
			.intent = AlcIntent_Query,
			.size = size,
			.align = ArenaAlc_Unit_size,
			.range = AlcRange_Loose,
			.relative = req_relative,
			.hint = hint,
			.flags = FLAG(AlcFlag, Zero),
			.offers_size = 2
		};

		auto res = Alc_invoke(provider, &request, offers, nullptr);
		switch (AlcRes_get(res)) {
			default: return res;
			case AlcRes_Ok:
		}

		for (uint i = 0; i < request.offers_size; i++) {
			if (offers[i] >= least_size) {
				size = offers[i];
				break;
			}
		}
	}

	AlcReq request = {
		.intent = AlcIntent_New,
		.size = size,
		.align = ArenaAlc_Unit_size,
		.relative = req_relative,
		.hint = hint,
		.flags = FLAG(AlcFlag, Zero)
	};

	ArenaAlc_Chunk *chunk = Alc_invoke(provider, &request, nullptr, nullptr);
	if (AlcRes_get(chunk)) // propagate error
		return chunk;

	size = request.size - sizeof(ArenaAlc_Chunk);

	auto units = (ArenaAlc_Units)(size / ArenaAlc_Unit_size);

	// example: when unit width = 64, we need a 1 unit bitmap for 64 memory units,
	// 	in total 65, so to get number of bitmap units from total size, we divide
	// 	the total number of units by unit_width + 1 (ceil)
	ArenaAlc_Units bitmap_units = (units + ArenaAlc_Unit_width) / (ArenaAlc_Unit_width + 1);

	auto bitmap = (ArenaAlc_Unit*)chunk->data;

	ArenaAlc_Units rest_units = units - bitmap_units;
	ArenaAlc_Units usable_units = bitmap_units * ArenaAlc_Unit_width;
	if (usable_units > rest_units) {
		// set last bits of bitmap to 1 to signify unusable memory
		bitmap[bitmap_units - 1] = (ArenaAlc_Unit)(~0ull) >> (usable_units - rest_units);
		usable_units = rest_units;
	}

	chunk->bitmap_units = bitmap_units;
	chunk->maxfree = usable_units;
	chunk->type = ArenaAlc_ChunkType_Bitmapped;

	return chunk;
}

Ptr ArenaAlc_Chunk_bmreserve(
	ArenaAlc_Chunk *chunk, usize size, ualign align, ArenaAlc_Units *mfp
) {
	if (align < ArenaAlc_Unit_size)
		align = ArenaAlc_Unit_size;

	auto bitmap_units = chunk->bitmap_units;
	usize ptr = (usize)chunk->data + bitmap_units * ArenaAlc_Unit_size;

	usize found_ptr = 0;
	usize found_size = usize_max;

	auto bitmap = (ArenaAlc_Unit*)chunk->data;
	u32 bits = bitmap_units * ArenaAlc_Unit_width;

	u64 val = *(bitmap++);
	u32 run = 0;
	u32 run_idx = 0;
	u32 i = 0;

	while (i < bits) {
		if (val == 0) {
			u32 d = ArenaAlc_Unit_width - (i % ArenaAlc_Unit_width);
			i += d;
			run += d;
			if (i < bits) {
				val = *(bitmap++);
				continue;
			} else {
				usize mem_ptr = ptr + (run_idx + 1) * ArenaAlc_Unit_size;
			}
		}

		u32 tz = (u32)__builtin_ctzg(val);
		run += tz;

		// one unit reserved for buffer header
		usize mem_ptr = ptr + (run_idx + 1) * ArenaAlc_Unit_size;
		usize end_ptr = ptr + (run_idx + run) * ArenaAlc_Unit_size;

		mem_ptr = usize_align(mem_ptr, align);

		while ((mem_ptr < end_ptr) && ((end_ptr - mem_ptr) >= size)) {
			found_ptr = mem_ptr;
			found_size = end_ptr - mem_ptr;
			mem_ptr += align;
		}

		val = ~(val >> tz);
		u32 tz2 = (u32)__builtin_ctzg(val);
		val = (~val) >> tz2;

		i += tz + tz2;
		run = 0;
		run_idx = i;
	}

	return nullptr;
}

Ptr ArenaAlc_Chunk_reserve(
	ArenaAlc_Chunk *chunk, usize size, ualign align, ArenaAlc_Units *mfp
) {
	Ptr result = nullptr;

	if (chunk->maxfree * ArenaAlc_Unit_size >= size) {
		switch (chunk->type) {
			default:
				PANIC();
			case ArenaAlc_ChunkType_Bitmapped:
				result = ArenaAlc_Chunk_bmreserve(chunk, size, align, mfp);
				break;
		}

		if (result) return result;
	}
}
