enum {
	FLAG_DEF(Trie_Branch),
	FLAG_DEF(Trie_Occupied),
	FLAG_DEF(Trie_Const),
};

constexpr u8 Trie_tag_bits = 3;
constexpr usize Trie_tag_mask = 0b111;

typedef struct {
	usize value;
} Trie;

constexpr Trie Trie_null = {};

bool Trie_isnull(Trie this) {
	return this.value == 0;
}

Ptr Trie_data(Trie this) {
	return (Ptr)(this.value & (~Trie_tag_mask));
}

Trie Trie_const(Trie this) {
	return (Trie){this.value | FLAG(Trie_Const)};
}

Trie Trie_ZZtag(Ptr data, u8 flags) {
	return (Trie){(usize)data | flags};
}

typedef struct {
	Ptr value;
	u64 map[4];
	Trie next[];
} TrieBranch;

static_assert(alignof(TrieBranch) > Trie_tag_mask);

// trie segment size type
typedef u16 TrieSize;

typedef struct {
	Ptr value;
	Trie next;
	TrieSize size;
	u8 bytes[];
} TrieSegment;

static_assert(alignof(TrieSegment) > Trie_tag_mask);
