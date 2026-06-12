#define HashMap_FLAG_BITS 2
#define HashMap_INFO_BITS 64
#define HashMap_HASH_BITS (HashMap_INFO_BITS - HashMap_FLAG_BITS)

#define HashMap_HASH_MASK  ((1ull << HashMap_HASH_BITS) - 1)

enum {
	HashMap_BIT_OCCUPIED = HashMap_HASH_BITS,
	HashMap_BIT_VACATED
};

typedef struct {
	u64 info;
	Ptr value;
} HashMapItem;

typedef struct {
	u64 info;
	HashMapItem *items;
} HashMap;
