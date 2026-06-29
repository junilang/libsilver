enum {
	FIELD_DEFINE(HashMapItem_Hash, uword_width - 2),
	HashMapItem_BIT_Occupied,
	HashMapItem_BIT_Vacated
};

typedef struct {
	usize metadata;
	Ptr value;
} HashMapItem;
