#ifndef HashMap_SAFE
	#define HashMap_SAFE BUILD_SAFE
#endif

typedef struct {
	usize hash;
	Ptr value;
} HashMapItem;

static_assert(alignof(HashMapItem) == alignof(usize));

enum {
	FIELD_DEF(HashMap_Power, 6),
	FIELD_DEF(HashMap_Empty, 58)
};

constexpr usize HashMap_empty_one = FIELD_SET(HashMap_Empty, 1);

constexpr u8 HashMap_sentinel_vacated = 0b1111'1111;
constexpr u8 HashMap_sentinel_empty = 0b1000'0000;
constexpr u8 HashMap_hash_mask = 0b0111'1111;

constexpr usize HashMap_search_size = sizeof(usize);
constexpr usize HashMap_size_base = HashMap_search_size;
constexpr usize HashMap_stencil_base = 0x0101010101010101ull;
constexpr usize HashMap_empty_stencil = HashMap_stencil_base * HashMap_sentinel_empty;

// ensure hashmap size is multiple of two
static_assert(
	(HashMap_size_base >= HashMap_search_size) &&
	(
		HashMap_size_base == 2 ||
		HashMap_size_base == 4 ||
		HashMap_size_base == 8 ||
		HashMap_size_base == 16 ||
		HashMap_size_base == 32 ||
		HashMap_size_base == 64 ||
		HashMap_size_base == 128 ||
		HashMap_size_base == 256
	)
);

typedef struct {
	usize info;
	Ptr data;
} HashMap;

typedef bool (*HashMap_MatchFn)(Ptr cond, Ptr value);

constexpr PrintFmt u64_bitsfmt = {
	FIELD(IntFmt_Base, Bin) |
	FIELD_SET(IntFmt_Digits, 64) |
	FIELD_SET(IntFmt_Spacing, 8)
};

AlcRes HashMap_expand(HashMap *this, Alc alc) {
	u8 power = FIELD_GET(HashMap_Power, this->info);
	auto const size = HashMap_size_base << power;
	auto const data = this->data;

	if (!data) { // initialize empty hashmap
		#if 0 && BUILD_DEBUG
			PRINTBP(128, Stdout, "hashmap_expand init size=",size,"\n");
		#endif

		AlcReq req = {
			.intent = AlcIntent_New,
			.align = alignof(usize),
			// size bytes for lookup table + HashMapItem[size] for data
			.size = size * (1 + sizeof(HashMapItem)),
			#if BUILD_DEBUG
				.flags = FLAG(AlcFlag_Zero),
			#endif
		};

		Ptr new_data = Alc_invoke(alc, &req, nullptr, nullptr);
		auto alc_res = AlcPtr_get(new_data);
		if (alc_res) return alc_res;

		// set lookup array to empty
		memset(new_data, HashMap_sentinel_empty, size);

		this->data = new_data;
		this->info = FIELD_SET(HashMap_Power, power) | FIELD_SET(HashMap_Empty, size / 2);

		return AlcRes_Ok;
	}

	power++;
	#if HashMap_SAFE
		if (power > FIELD_MAX(HashMap_Power))
			PANIC("power overflow");
	#endif

	usize const new_size = HashMap_size_base << power;

	usize const used = (size / 2) - FIELD_GET(HashMap_Empty, this->info);
	usize const empty = (new_size / 2) - used;

	#if 0 && BUILD_DEBUG
		PRINTBP(128, Stdout, "hashmap_expand grow (",used,") ",size," -> ",new_size,"\n");
	#endif

	AlcReq req = {
		.intent = AlcIntent_New,
		.align = alignof(usize),
		.size = new_size * (1 + sizeof(HashMapItem)),
		#if BUILD_DEBUG
			.flags = FLAG(AlcFlag_Zero),
		#endif
	};

	Ptr new_data = Alc_invoke(alc, &req, nullptr, nullptr);
	auto alc_res = AlcPtr_get(new_data);
	if (alc_res) return alc_res;

	memset(new_data, HashMap_sentinel_empty, new_size);

	auto src_table = (usize*)data;
	auto const src_table_end = (usize*)((ubyte*)data + size);

	usize *const table_end = (usize*)((ubyte*)new_data + new_size);

	usize src_offset = 0;
	// loop over items in hashmap and add them to the new data buffer
	for (; src_table < src_table_end; src_table++) {
		// leaves top bits of non empty slots
		usize src_lookup = (~(*src_table)) & (HashMap_stencil_base * HashMap_sentinel_empty);

		#if 0 && BUILD_DEBUG
			PRINTBP(512, Stderr, u64_bitsfmt, *src_table, "\n", u64_bitsfmt, src_lookup, "\n");
		#endif

		while (src_lookup) {
			auto const i = src_offset + (
				((usize)__builtin_ctzg(src_lookup) - (ubyte_width - 1)) / ubyte_width
			);

			src_lookup &= (src_lookup - 1);

			#if 0 && BUILD_DEBUG
				PRINTBP(128, Stderr, "i=", i, "\n");
			#endif

			auto const src_item = ((HashMapItem*)src_table_end)[i];

			// find slot for item in new table

			usize const search_index = (src_item.hash & (new_size - 1)) / sizeof(usize);
			auto const table_begin = (usize*)new_data + search_index;
			auto table = table_begin;

			usize result;
			for (; table < table_end; table++) {
				result = (*table) & HashMap_empty_stencil;
				if (result) goto found;
			}

			table = (usize*)new_data;
			for (; table < table_begin; table++) {
				result = (*table) & HashMap_empty_stencil;
				if (result) goto found;
			}

			UNREACHABLE;

			found:;
			auto const entry_offset =
				((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width
			;

			auto const table_entry = (ubyte*)table + entry_offset;
			*table_entry = src_item.hash & HashMap_hash_mask;

			auto item = (HashMapItem*)table_end + (table_entry - (ubyte*)new_data);
			*item = src_item;
		}

		src_offset += sizeof(usize);
	}

	this->data = new_data;
	this->info = FIELD_SET(HashMap_Power, power) | FIELD_SET(HashMap_Empty, empty);

	// free previous data buffer
	return Alc_delete(alc, data);
}

AlcPtr HashMap_add(HashMap *this, usize hash, Alc alc) {
	// expand hashmap if no empty slots remain
	if (this->info < FIELD_SET(HashMap_Empty, 1)) {
		AlcRes err = HashMap_expand(this, alc);
		if (err) return AlcPtr_set(err);
	}

	auto const data = this->data;
	auto const size = HashMap_size_base << FIELD_GET(HashMap_Power, this->info);

	usize const search_index = (hash & (size - 1)) / sizeof(usize);

	auto const table_begin = (usize*)data + search_index;
	auto const table_end = (usize*)((ubyte*)data + size);

	usize result;

	auto table = table_begin;
	for (; table < table_end; table++) {
		result = (*table) & HashMap_empty_stencil;
		if (result) goto found;
	}

	table = (usize*)data;
	for (; table < table_begin; table++) {
		result = (*table) & HashMap_empty_stencil;
		if (result) goto found;
	}

	// something went wrong and there are no empty slots
	UNREACHABLE;

	found:;
	auto const entry_offset = ((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width;
	auto const table_entry = (ubyte*)table + entry_offset;

	*table_entry = hash & HashMap_hash_mask;

	auto item = (HashMapItem*)table_end + (table_entry - (ubyte*)data);
	item->hash = hash;
	this->info -= FIELD_SET(HashMap_Empty, 1);

	return (AlcPtr)&item->value;
}

Ptr HashMap_get(HashMap *this, usize hash, HashMap_MatchFn fn, Ptr cond) {
	auto const data = this->data;
	if (!data) return nullptr;

	auto const size = HashMap_size_base << FIELD_GET(HashMap_Power, this->info);
	auto const items = (HashMapItem*)((ubyte*)data + size);

	usize const search_index = (hash & (size - 1)) / sizeof(usize);
	auto const stencil = HashMap_stencil_base * (hash & HashMap_hash_mask);

	auto const table = (usize*)data;
	auto const index_begin = search_index;
	auto const index_end = size / sizeof(usize);

	#define ZZTEST { \
		usize result = table[index] ^ stencil; \
		result = (result - HashMap_stencil_base) & (~result) & HashMap_empty_stencil; \
		usize base_index = index * ubyte_width; \
		while (result) { \
			usize i = base_index + ( \
				(((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width) \
			); \
			HashMapItem *item = &items[i]; \
			if (item->hash == hash && fn(cond, item->value)) \
				return &item->value; \
			result &= (result - 1); \
		} \
		result = table[index] ^ HashMap_empty_stencil; \
		result = (result - HashMap_stencil_base) & (~result) & HashMap_empty_stencil; \
		if (result) return nullptr; \
	}

	auto index = index_begin;
	for (; index < index_end; index++) ZZTEST

	// wrap around
	index = 0;
	for (; index < index_begin; index++) ZZTEST

	#undef ZZTEST

	return nullptr;
}

void HashMap_delete(HashMap *this, Ptr value_ptr) {
	auto const item = (HashMapItem*)(
		(ubyte*)value_ptr - __builtin_offsetof(HashMapItem, value)
	);

	auto const size = HashMap_size_base << FIELD_GET(HashMap_Power, this->info);
	auto const items = (HashMapItem*)((ubyte*)this->data + size);

	#if HashMap_SAFE
		auto const items_end = items + size;
		if (item < items || item >= items_end)
			PANIC("item out of bounds");
	#endif

	auto const index = (usize)(item - items);

	auto const table = (ubyte*)this->data;
	table[index] = HashMap_sentinel_vacated;

	this->info += FIELD_SET(HashMap_Empty, 1);
}
