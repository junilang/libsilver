#define FIELD_DEF(name, width) name##_FIELD, name##_FIELD_END = name##_FIELD + (width - 1)

#define FIELD_WIDTH(field) \
	((field##_FIELD_END - field##_FIELD) + 1)

#define FIELD_MAX(field) ( \
	(FIELD_WIDTH(field) >= LLONG_WIDTH) ? \
		(~0ull) : \
		((1ull << FIELD_WIDTH(field)) - 1) \
)

#define FIELD_MASK(field) (FIELD_MAX(field) << field##_FIELD)

#define FIELD_CLEAR(field) (~FIELD_MASK(field))

#define FIELD_GET(field, from) \
	(((ullong)(from) & FIELD_MASK(field)) >> field##_FIELD)

#define FIELD_GET_CAST(field, from) \
	((field)FIELD_GET(field, from))

#define FIELD_SET(field, val) \
	(((ullong)(val) << field##_FIELD) & FIELD_MASK(field))

#define FIELD(field, member) FIELD_SET(field, field##_##member)
