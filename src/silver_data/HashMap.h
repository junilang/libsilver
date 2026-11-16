#ifndef HashMap_SAFE
	#define HashMap_SAFE LIBSILVER_SAFE
#endif

#ifndef HashMap_DEBUG
	#define HashMap_DEBUG false
#endif

typedef u64 HashMapInfo; enum {
	FIELD_DEF(HashMap_Power, 6),
	FIELD_DEF_UNTIL(HashMap_Empty, 64),
	HashMapInfo_width
};

static_assert(HashMapInfo_width <= 64);

usize constexpr HashMap_Empty_one = FIELD_SET(HashMap_Empty, 1);

#ifndef HashMap_ALIGN
	#define HashMap_ALIGN _Alignof(usize)
#endif

ualign constexpr HashMap_align = HashMap_ALIGN;

ubyte constexpr HashMap_svacated = 0b1111'1111;
ubyte constexpr HashMap_sempty = 0b1000'0000;
ubyte constexpr HashMap_shashmask = 0b0111'1111;

auto constexpr HashMap_searchsize = sizeof(usize);
usize constexpr HashMap_stencil = 0x0101010101010101ull;
usize constexpr HashMap_stencil_empty = HashMap_stencil * HashMap_sempty;

static_assert(usize_width == 64);
static_assert(ubyte_width == 8);

#ifndef HashMap_BASESIZE
	#define HashMap_BASESIZE HashMap_searchsize
#endif

usize constexpr HashMap_basesize = HashMap_BASESIZE;

static_assert(HashMap_basesize >= HashMap_searchsize);
static_assert(
	(HashMap_basesize & (HashMap_basesize - 1)) == 0
	, "basesize is not power of 2"
);

typedef struct {
	HashMapInfo info;
	Ptr data;
} HashMap;

typedef struct {
	usize hash;
	Ptr value;
} HashMapItem;

static_assert(alignof(HashMapItem) == alignof(usize));

typedef bool (*HashMap_MatchFn)(Ptr cond, Ptr value);
typedef Ptr (*HashMap_IterFn)(Ptr payload, usize hash, Ptr value);

// determines the load factor, currently set to 50%
usize HashMap_slots_(usize size) {
	return size / 2;
}

auto constexpr HashMap_maxslots = usize_max;

void HashMap_upgrade_(Ptr src_data, usize src_size, Ptr dst_data, usize dst_size) {
	auto src_table = (usize*)src_data;
	auto const src_table_end = (usize*)((usize)src_data + src_size);

	auto const table_end = (usize*)((usize*)dst_data + dst_size);

	usize src_offset = 0;
	// loop over items in hashmap and add them to the new data buffer
	for (; src_table < src_table_end; src_table++) {
		// leaves top bits of non empty slots
		usize src_lookup = (~(*src_table)) & HashMap_stencil_empty;

		while (src_lookup) {
			auto const i = src_offset + (
				((usize)__builtin_ctzg(src_lookup) - (ubyte_width - 1)) / ubyte_width
			);

			src_lookup &= (src_lookup - 1);

			#if HashMap_DEBUG
				PRINTA(Stderr, "i=", i, "\n");
			#endif

			auto const src_item = ((HashMapItem*)src_table_end)[i];

			// find slot for item in new table

			usize const search_index = (src_item.hash & (dst_size - 1)) / sizeof(usize);
			auto const table_begin = (usize*)dst_data + search_index;
			auto table = table_begin;

			usize result;
			for (; table < table_end; table++) {
				result = (*table) & HashMap_stencil_empty;
				if (result) goto found;
			}

			table = (usize*)dst_data;
			for (; table < table_begin; table++) {
				result = (*table) & HashMap_stencil_empty;
				if (result) goto found;
			}

			UNREACHABLE(HashMap_SAFE);

			found:;
			auto const entry_offset =
				((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width
			;

			auto const table_entry = (ubyte*)table + entry_offset;
			*table_entry = src_item.hash & HashMap_shashmask;

			auto item = (HashMapItem*)table_end + ((usize)table_entry - (usize)dst_data);
			*item = src_item;
		}

		src_offset += sizeof(usize);
	}
}

[[nodiscard]]
AlcRes HashMap_init_(HashMap *this, Alc alc, u8 power) {
	auto const size = HashMap_basesize << power;

	auto const ptr = Alc_invoke(alc,
		AlcReq_new(size * (1 + sizeof(HashMapItem)), HashMap_align, HashMap_DEBUG)
	, nullptr, nullptr);
	auto const res = AlcPtr_get(ptr);
	if (res) return res;

	auto const new_data = (Ptr)ptr;
	memset(new_data, HashMap_sempty, size);

	this->data = new_data;
	this->info =
		FIELD_SET(HashMap_Power, power) |
		FIELD_SET(HashMap_Empty, HashMap_slots_(size))
	;

	return AlcRes_Ok;
}

[[nodiscard]]
AlcRes HashMap_expand_(HashMap *this, Alc alc, u8 new_power) {
	auto const info = this->info;

	u8 const power = FIELD_GET(HashMap_Power, info);
	#if HashMap_SAFE
		ASSERT(new_power >= power);
		ASSERT(new_power <= FIELD_MAX(HashMap_Power));
	#endif

	auto const size = HashMap_basesize << power;
	auto const used = HashMap_slots_(size) - FIELD_GET(HashMap_Empty, info);

	auto const data = this->data;

	auto const new_size = HashMap_basesize << new_power;
	auto const new_empty = HashMap_slots_(new_size) - used;

	HashMapInfo const new_info =
		FIELD_SET(HashMap_Power, new_power) |
		FIELD_SET(HashMap_Empty, new_empty)
	;

	auto const ptr = Alc_invoke(alc,
		AlcReq_new(new_size * (1 + sizeof(HashMapItem)), HashMap_align, HashMap_DEBUG)
	, nullptr, nullptr);
	auto const res = AlcPtr_get(ptr);
	if (res) return res;

	auto const new_data = (Ptr)ptr;
	memset(new_data, HashMap_sempty, new_size);
	HashMap_upgrade_(data, size, new_data, new_size);

	this->data = new_data;
	this->info = new_info;

	return AlcRes_Ok;
}

[[nodiscard]]
AlcRes HashMap_init(HashMap *this, Alc alc, usize slots) {
	#if HashMap_SAFE
		ASSERT(slots <= HashMap_maxslots);
	#endif

	u8 power = 0;
	usize size = HashMap_basesize;
	while (HashMap_slots_(size) < slots) {
		power++;
		size <<= 1;
	}

	return HashMap_init_(this, alc, power);
}

[[nodiscard]]
AlcRes HashMap_destroy(HashMap *this, Alc alc) {
	auto const data = this->data;
	AlcRes res;
	if (data)
		res = Alc_delete(alc, data);
	else {
		res = AlcRes_Ok;
	}

	#if HashMap_SAFE
		this->data = nullptr;
		this->info = 0;
	#endif

	return res;
}

[[nodiscard]]
AlcRes HashMap_defragment(HashMap *this, Alc alc) {
	// allocates a new buffer and runs the reinsertion algorithm
	return HashMap_expand_(this, alc, FIELD_GET(HashMap_Power, this->info));
}

[[nodiscard]]
AlcRes HashMap_reserve(HashMap *this, Alc alc, usize slots) {
	if (!this->data)
		return HashMap_init(this, alc, slots);

	auto const info = this->info;
	u8 power = FIELD_GET(HashMap_Power, info);
	auto empty = FIELD_GET(HashMap_Empty, info);
	auto size = HashMap_basesize << power;

	// there are already enough empty slots
	if (slots <= empty)
		return AlcRes_Ok;

	auto const used = HashMap_slots_(size) - empty;

	do {
		power++;
		#if HashMap_SAFE
			ASSERT(power <= FIELD_MAX(HashMap_Power));
		#endif

		size = HashMap_basesize << power;
		empty = HashMap_slots_(size) - used;
	} while (slots > empty);

	return HashMap_expand_(this, alc, power);
}

Ptr HashMap_add_unsafe(HashMap *this, usize hash) {
	auto const data = this->data;
	auto const info = this->info;
	auto const size = HashMap_basesize << FIELD_GET(HashMap_Power, info);

	usize const search_index = (hash & (size - 1)) / sizeof(usize);

	auto const table_begin = (usize*)data + search_index;
	auto const table_end = (usize*)((usize)data + size);

	usize result;

	auto table = table_begin;
	for (; table < table_end; table++) {
		result = (*table) & HashMap_stencil_empty;
		if (result) goto found;
	}

	table = (usize*)data;
	for (; table < table_begin; table++) {
		result = (*table) & HashMap_stencil_empty;
		if (result) goto found;
	}

	// something went wrong and there are no empty slots
	UNREACHABLE(HashMap_SAFE);

	found:;
	auto const entry_offset = ((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width;
	auto const table_entry = (ubyte*)table + entry_offset;

	*table_entry = hash & HashMap_shashmask;

	auto const item = (HashMapItem*)table_end + (table_entry - (ubyte*)data);
	item->hash = hash;
	this->info -= HashMap_Empty_one;

	return &item->value;
}

void HashMap_bulkadd_unsafe(HashMap *this, const HashMapItem *items, usize items_size) {
	auto const data = this->data;
	auto const size = HashMap_basesize << FIELD_GET(HashMap_Power, this->info);

	auto const table_end = (usize*)((usize)data + size);
	auto const items_end = items + items_size;

	for (; items < items_end; items++) {
		HashMapItem item = *items;

		usize const search_index = (item.hash & (size - 1)) / sizeof(usize);
		auto const table_begin = (usize*)data + search_index;

		usize result;

		auto table = table_begin;
		for (; table < table_begin; table++) {
			result = (*table) & HashMap_stencil_empty;
			if (result) goto found;
		}

		table = (usize*)data;
		for (; table < table_begin; table++) {
			result = (*table) & HashMap_stencil_empty;
			if (result) goto found;
		}

		UNREACHABLE(HashMap_SAFE);

		found:;
		auto const entry_offset = ((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width;
		auto const table_entry = (ubyte*)table + entry_offset;

		*table_entry = item.hash & HashMap_shashmask;

		auto const itemp = (HashMapItem*)table_end + (table_entry - (ubyte*)data);
		*itemp = item;
	}

	this->info -= FIELD_SET(HashMap_Empty, items_size);
}

[[nodiscard]]
AlcRes HashMap_bulkadd(HashMap *this, Alc alc, const HashMapItem *items, usize items_size) {
	auto const res = HashMap_reserve(this, alc, items_size);
	if (res) return res;

	HashMap_bulkadd_unsafe(this, items, items_size);
	return AlcRes_Ok;
}

[[nodiscard]]
AlcPtr HashMap_add(HashMap *this, Alc alc, usize hash) {
	auto const data = this->data;
	if (!data) {
		auto const res = HashMap_init_(this, alc, 0);
		if (res) return AlcPtr_set(res);
	}

	auto const info = this->info;
	// expand hashmap if no empty slots remain
	if (info < HashMap_Empty_one) {
		u8 const power = FIELD_GET(HashMap_Power, info);
		auto const res = HashMap_expand_(this, alc, power + 1);
		if (res) return AlcPtr_set(res);
	}

	return (AlcPtr)HashMap_add_unsafe(this, hash);
}

Ptr HashMap_get(HashMap *this, usize hash, HashMap_MatchFn fn, Ptr cond) {
	auto const data = this->data;
	if (!data) return nullptr;

	auto const info = this->info;

	auto const size = HashMap_basesize << FIELD_GET(HashMap_Power, info);
	auto const table_end = (usize*)((usize)data + size);

	usize const search_index = (hash & (size - 1)) / sizeof(usize);
	auto const table_begin = (usize*)((usize)data + search_index);

	auto const stencil = HashMap_stencil * (hash & HashMap_shashmask);

	#define ZZTEST { \
		auto const entry = *table; \
		usize result = entry ^ stencil; \
		result = (result - HashMap_stencil) & (~result) & HashMap_stencil_empty; \
		\
		if (!result) continue; \
		auto const base_index = (usize)table - (usize)data; \
		while (result) { \
			usize const i = base_index + ( \
				((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width \
			); \
			\
			auto item = (HashMapItem*)table_end + i; \
			if (item->hash == hash && fn(cond, item->value)) \
				return &item->value; \
			\
			result &= (result - 1); \
		} \
		\
		result = entry ^ HashMap_stencil_empty; \
		result = (result - HashMap_stencil) & (~result) & HashMap_stencil_empty; \
		if (result) return nullptr; \
	}

	auto table = table_begin;
	for (; table < table_end; table++) ZZTEST;

	table = (usize*)data;
	for (; table < table_begin; table++) ZZTEST;

	#undef ZZTEST

	return nullptr;
}

Ptr HashMap_iter(HashMap *this, HashMap_IterFn fn, Ptr payload) {
	auto const data = this->data;
	if (!data) return nullptr;

	auto const info = this->info;

	auto const size = HashMap_basesize << FIELD_GET(HashMap_Power, info);

	auto const table_end = (usize*)((usize)data + size);
	auto table = (usize*)data;

	usize base_index = 0;
	for (; table < table_end; table++) {
		usize result = (~(*table)) & HashMap_stencil_empty;
		while (result) {
			usize const i = base_index + (
				((usize)__builtin_ctzg(result) - (ubyte_width - 1)) / ubyte_width
			);

			auto const item = (const HashMapItem*)table_end + i;
			Ptr const res = fn(payload, item->hash, item->value);
			if (res) return res;

			result &= (result - 1);
		}

		base_index += sizeof(usize);
	}

	return nullptr;
}

void HashMap_delete(HashMap *this, Ptr value_ptr) {
	#if HashMap_SAFE
		ASSERT(this->data, "HashMap not initialized");
	#endif

	auto const item = (HashMapItem*)(
		(ubyte*)value_ptr - __builtin_offsetof(HashMapItem, value)
	);

	auto const data = (ubyte*)this->data;
	auto const info = this->info;

	auto const size = HashMap_basesize << FIELD_GET(HashMap_Power, info);
	auto const items = (HashMapItem*)((usize)data + size);

	#if HashMap_SAFE
		auto const items_end = items + size;
		ASSERT(item >= items && item < items_end, "item out of bounds");
		item->hash = 0;
		item->value = nullptr;
	#endif

	auto const i = (usize)(item - items);
	data[i] = HashMap_svacated;

	this->info += FIELD_SET(HashMap_Empty, 1);
}
