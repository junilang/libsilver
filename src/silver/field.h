#define FIELD_LEN(field) \
	((field##_END - field##_FIELD) + 1)

#define FIELD_GET(field, from) \
	(((usize)(from) >> field##_FIELD) & ((1ull << FIELD_LEN(field)) - 1))

#define FIELD_CGET(field, from) \
	((field)FIELD_GET(field, from))

#define FIELD_SET(field, member) \
	(((usize)(field##_##member) & ((1ull << FIELD_LEN(field)) - 1)) << field##_FIELD)

#define FIELD_SETN(field, val) \
	(((usize)(val) & ((1ull << FIELD_LEN(field)) - 1)) << field##_FIELD)

#define FIELD_CLEAR(field) \
	(~(((1ull << FIELD_LEN(field)) - 1) << field##_FIELD))
