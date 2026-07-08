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

constexpr usize HashMap_size_base = 8;

typedef struct {
	usize info;
	Ptr data;
} HashMap;

AlcRes HashMap_expand(HashMap *this, Alc alc) {
	auto const size = HashMap_size_base << FIELD_GET(HashMap_Power, this->info);
	auto const data = this->data;
	if (!data) {
		AlcReq req = {
			.intent = AlcIntent_New,
			.align = alignof(usize),
			// size bytes for lookup table + HashMapItem[size] for data
			.size = size * (1 + sizeof(HashMapItem)),
		};

		Ptr new_data = Alc_invoke(alc, &req, nullptr, nullptr);
		auto alc_res = AlcPtr_get(new_data);
		if (alc_res) return alc_res;

		// set lookup table to empty
		memset(new_data, HashMap_sentinel_empty, size);

		this->data = new_data;

		return AlcRes_Ok;
	}

	return AlcRes_ErrUnimplemented;
}
