#define FIELD_DEF(field, width) \
	field##_FIELD, field##_FIELD_END = field##_FIELD + ((width) - 1)

#define FIELD_DEF_FROM(field, begin, width) \
	field##_FIELD = (begin), field##_FIELD_END = field##_FIELD + ((width) - 1)

#define FIELD_DEF_UNTIL(field, end) \
	field##_FIELD, field##_FIELD_END = ((end) - 1)

#define FIELD_DEF_BACK(field, end, width) \
	field##_FIELD = ((end) - (width)), field##_FIELD_END = ((end) - 1)

#define FIELD_DEF_RANGE(field, begin, end) \
	field##_FIELD = (begin), field##_FIELD_END = ((end) - 1)

#define FIELD_DEF_UPPER(field, parent, width) \
	field##_FIELD = ((parent##_FIELD_END + 1) - (width)), field##_FIELD_END = parent##_FIELD_END

#define FIELD_DEF_LOWER(field, parent, width) \
	field##_FIELD = parent##_FIELD, field##_FIELD_END = field##_FIELD + ((width) - 1), \
	field_FIELD_PARENT_END = parent##_FIELD_END

#define FIELD_MAX(field) ( \
	(FIELD_WIDTH(field) >= LLONG_WIDTH) ? \
		(~0ull) : \
		((1ull << FIELD_WIDTH(field)) - 1) \
)

#define FIELD_BEGIN(field) field##_FIELD
#define FIELD_WIDTH(field) ((field##_FIELD_END + 1) - FIELD_BEGIN(field))

#define FIELD_MASK(field) ( \
	(FIELD_WIDTH(field) >= LLONG_WIDTH) ? \
		(~0ull) : \
		(((1ull << FIELD_WIDTH(field)) - 1) << FIELD_BEGIN(field)) \
)

#define FIELD_MASK_UPPER(field, bits) ( \
	(((bits) >= FIELD_WIDTH(field)) || ((bits) >= LLONG_WIDTH)) ? \
		FIELD_MASK(field) : \
		((FIELD_MASK(field) << (FIELD_WIDTH(field) - (bits))) & FIELD_MASK(field)) \
)

#define FIELD_MASK_LOWER(field, bits) ( \
	(((bits) >= FIELD_WIDTH(field)) || ((bits) >= LLONG_WIDTH)) ? \
		FIELD_MASK(field) : \
		((FIELD_MASK(field) >> (FIELD_WIDTH(field) - (bits))) & FIELD_MASK(field)) \
)

#define FIELD_CLEAR(field) (~FIELD_MASK(field))
#define FIELD_CLEAR_UPPER(field, bits) (~FIELD_MASK_UPPER(field, (bits)))
#define FIELD_CLEAR_LOWER(field, bits) (~FIELD_MASK_LOWER(field, (bits)))

#define FIELD_GET(field, from) \
	(((ullong)(from) & FIELD_MASK(field)) >> field##_FIELD)

#define FIELD_GET_CAST(field, from) \
	((field)FIELD_GET(field, (from)))

#define FIELD_SET(field, value) \
	(((ullong)(value) << FIELD_BEGIN(field)) & FIELD_MASK(field))

#define FIELD(field, member) FIELD_SET(field, field##_##member)

enum { FIELD_DEF(u8, 8) };
enum { FIELD_DEF(u16, 16) };
enum { FIELD_DEF(u32, 32) };
enum { FIELD_DEF(u64, 64) };
