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

#ifndef HashMap_ALIGN
	#define HashMap_ALIGN alignof(usize)
#endif

constexpr ualign HashMap_align = HashMap_ALIGN;

#ifndef HashMap_SIZE_BASE
	#define HashMap_SIZE_BASE sizeof(usize)
#endif

constexpr usize HashMap_size_base = HashMap_SIZE_BASE;

constexpr usize HashMap_search_size = sizeof(usize);
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

// determines the load factor, currently set to 50%
usize HashMap_ZZslots(usize size) {
	return size / 2;
}

u8 HashMap_ZZpowerfor(u8 power, usize slots) {
	while (HashMap_ZZslots(HashMap_size_base << power) < slots) {
		power++;
	}
	return power;
}

void HashMap_ZZupgrade(Ptr src_data, usize src_size, Ptr dst_data, usize dst_size) {
	auto src_table = (usize*)src_data;
	auto const src_table_end = (usize*)((ubyte*)src_data + src_size);

	usize *const table_end = (usize*)((ubyte*)dst_data + dst_size);

	usize src_offset = 0;
	// loop over items in hashmap and add them to the new data buffer
	for (; src_table < src_table_end; src_table++) {
		// leaves top bits of non empty slots
		usize src_lookup = (~(*src_table)) & HashMap_empty_stencil;

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

			usize const search_index = (src_item.hash & (dst_size - 1)) / sizeof(usize);
			auto const table_begin = (usize*)dst_data + search_index;
			auto table = table_begin;

			usize result;
			for (; table < table_end; table++) {
				result = (*table) & HashMap_empty_stencil;
				if (result) goto found;
			}

			table = (usize*)dst_data;
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

			auto item = (HashMapItem*)table_end + (table_entry - (ubyte*)dst_data);
			*item = src_item;
		}

		src_offset += sizeof(usize);
	}
}

[[nodiscard]]
AlcRes HashMap_ZZinit(HashMap *this, Alc alc, u8 power) {
	auto const size = HashMap_size_base << power;

	AlcReq req = {
		.intent = AlcIntent_New,
		.align = alignof(usize),
		.size = size * (1 + sizeof(HashMapItem)),
		#if BUILD_DEBUG
			.flags = FLAG(AlcFlag_Zero),
		#endif
	};

	Ptr new_data = Alc_invoke(alc, &req, nullptr, nullptr);
	auto res = AlcPtr_get(new_data);
	if (res) return res;

	memset(new_data, HashMap_sentinel_empty, size);

	this->data = new_data;
	this->info =
		FIELD_SET(HashMap_Power, power) |
		FIELD_SET(HashMap_Empty, HashMap_ZZslots(size))
	;

	return AlcRes_Ok;
}

[[nodiscard]]
AlcRes HashMap_ZZexpand(HashMap *this, Alc alc, u8 new_power) {
	auto const info = this->info;

	u8 const power = FIELD_GET(HashMap_Power, info);
	#if HashMap_SAFE
		if (new_power < power) // allows for defragmenting
			PANIC("new_power lower than old");
		if (new_power > FIELD_MAX(HashMap_Power))
			PANIC("new_power too high");
	#endif

	auto const size = HashMap_size_base << power;
	auto const used = HashMap_ZZslots(size) - FIELD_GET(HashMap_Empty, info);

	auto const data = this->data;

	auto const new_size = HashMap_size_base << new_power;
	auto const new_empty = HashMap_ZZslots(new_size) - used;

	usize const new_info =
		FIELD_SET(HashMap_Power, new_power) |
		FIELD_SET(HashMap_Empty, new_empty)
	;

	AlcReq req = {
		.intent = AlcIntent_New,
		.align = alignof(usize),
		.size = new_size * (1 + sizeof(HashMapItem)),
		#if BUILD_DEBUG
			.flags = FLAG(AlcFlag_Zero)
		#endif
	};

	Ptr new_data = Alc_invoke(alc, &req, nullptr, nullptr);
	auto const alc_res = AlcPtr_get(new_data);
	if (alc_res) return alc_res;

	memset(new_data, HashMap_sentinel_empty, new_size);

	HashMap_ZZupgrade(data, size, new_data, new_size);

	this->data = new_data;
	this->info = new_info;

	return AlcRes_Ok;
}

[[nodiscard]]
AlcRes HashMap_init(HashMap *this, Alc alc, usize slots) {
	u8 power = 0;
	usize size = HashMap_size_base;
	while (HashMap_ZZslots(size) < slots) {
		power++;
		#if HashMap_SAFE
			if (power > FIELD_MAX(HashMap_Power))
				PANIC("power overflow");
		#endif

		size <<= 1;
	}

	return HashMap_ZZinit(this, alc, power);
}

[[nodiscard]]
AlcRes HashMap_defragment(HashMap *this, Alc alc) {
	// allocates a new buffer and runs the reinsertion algorithm
	return HashMap_ZZexpand(this, alc, FIELD_GET(HashMap_Power, this->info));
}

[[nodiscard]]
AlcRes HashMap_reserve(HashMap *this, Alc alc, usize slots) {
	auto const info = this->info;

	u8 power = FIELD_GET(HashMap_Power, info);

	auto size = HashMap_size_base << power;
	auto empty = FIELD_GET(HashMap_Empty, info);
	if (slots <= empty) {
		if (!this->data)
			return HashMap_ZZinit(this, alc, power);
		else
			return AlcRes_Ok;
	}

	auto const used = HashMap_ZZslots(size) - empty;

	do {
		power++;
		#if HashMap_SAFE
			if (power > FIELD_MAX(HashMap_Power))
				PANIC("power overflow");
		#endif

		size <<= 1;
		empty = HashMap_ZZslots(size) - used;
	} while (slots > empty);

	if (!this->data)
		return HashMap_ZZinit(this, alc, power);
	else
		return HashMap_ZZexpand(this, alc, power);
}

Ptr HashMap_add_unsafe(HashMap *this, usize hash) {
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

	return &item->value;
}

void HashMap_bulkadd_unsafe(HashMap *this, const HashMapItem *items, usize items_size) {
	auto const data = this->data;
	auto const size = HashMap_size_base << FIELD_GET(HashMap_Power, this->info);

	auto const table_end = (usize*)((ubyte*)data + size);
	auto const items_end = items + items_size;

	for (; items < items_end; items++) {
		HashMapItem item = *items;

		usize const search_index = (item.hash & (size - 1)) / sizeof(usize);
		auto const table_begin = (usize*)data + search_index;

		usize result;

		auto table = table_begin;
		for (; table < table_begin; table++) {
			result = (*table) & HashMap_empty_stencil;
			if (result) goto found;
		}

		table = (usize*)data;
		for (; table < table_begin; table++) {
			result = (*table) & HashMap_empty_stencil;
			if (result) goto found;
		}

		UNREACHABLE;

		found:;
		auto const entry_offset = ((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width;
		auto const table_entry = (ubyte*)table + entry_offset;

		*table_entry = item.hash & HashMap_hash_mask;

		auto const itemp = (HashMapItem*)table_end + (table_entry - (ubyte*)data);
		*itemp = item;
	}

	this->info -= FIELD_SET(HashMap_Empty, items_size);
}

[[nodiscard]]
AlcRes HashMap_bulkadd(HashMap *this, Alc alc, const HashMapItem *items, usize items_size) {
	auto res = HashMap_reserve(this, alc, items_size);
	if (res) return res;

	HashMap_bulkadd_unsafe(this, items, items_size);
	return AlcRes_Ok;
}

[[nodiscard]]
AlcPtr HashMap_add(HashMap *this, usize hash, Alc alc) {
	// expand hashmap if no empty slots remain
	if (this->info < FIELD_SET(HashMap_Empty, 1)) {
		u8 power = FIELD_GET(HashMap_Power, this->info);
		AlcRes res;
		if (!this->data)
			res = HashMap_ZZinit(this, alc, power);
		else
			res = HashMap_ZZexpand(this, alc, power + 1);

		if (res) return AlcPtr_set(res);
	}

	return (AlcPtr)HashMap_add_unsafe(this, hash);
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

		item->hash = 0;
		item->value = nullptr;
	#endif

	auto const index = (usize)(item - items);

	auto const table = (ubyte*)this->data;
	table[index] = HashMap_sentinel_vacated;

	this->info += FIELD_SET(HashMap_Empty, 1);
}
