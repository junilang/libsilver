enum {
	FIELD_DEF(HashMapItem_Hash, usize_width - 2),
	FLAG_DEF(HashMapItem_Occupied),
	FLAG_DEF(HashMapItem_Vacated),
};

typedef struct {
	usize metadata;
	Ptr value;
} HashMapItem;

typedef struct {
	usize metadata;
} HashMap;
