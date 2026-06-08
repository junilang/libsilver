typedef u64 uhash;
constexpr uhash uhash_max = u64_max;
constexpr u8 uhash_width = 64;


#define FNV1a_BASE 0xcbf29ce484222325ULL
#define FNV1a_PRIME 0x100000001b3ULL

uhash memhash(ConstPtr data, usize size, uhash base) {
	const ubyte *bytes = data;
	for (usize i = 0; i < size; i++) {
		base = (base ^ bytes[i]) * FNV1a_PRIME;
	}
	return base;
}

uhash u8_hash(u8 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash u16_hash(u16 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash u32_hash(u32 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash u64_hash(u64 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash Ptr_hash(Ptr value, uhash base) {
	return (base ^ (usize)value) * FNV1a_PRIME;
}

#define HASH_COMBINE u64_hash

uhash hash_combine(uhash a, uhash b) {
	return HASH_COMBINE(a, b);
}

#define HASH_BASE FNV1a_BASE

constexpr uhash hash_base = HASH_BASE;

#define HASH(value, base) _Generic((value), \
	u8 : u8_hash, \
	u16 : u16_hash, \
	u32 : u32_hash, \
	u64 : u64_hash, \
	Ptr : Ptr_hash, \
	String : String_hash, \
	StringSpan : StringSpan_hash, \
	SmallString : SmallString_hash, \
	default : HASH_Ptr \
)(value, base)
