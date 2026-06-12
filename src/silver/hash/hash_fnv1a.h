constexpr u64 fnv1a_base = 0xcbf29ce484222325ull;
constexpr u64 fnv1a_prime = 0x100000001b3ull;

u64 u8_fnv1a(u64 base, u8 value) {
	return (base ^ value) * fnv1a_prime;
}

u64 u16_fnv1a(u64 base, u16 value) {
	return (base ^ value) * fnv1a_prime;
}

u64 u32_fnv1a(u64 base, u32 value) {
	return (base ^ value) * fnv1a_prime;
}

u64 u64_fnv1a(u64 base, u64 value) {
	return (base ^ value) * fnv1a_prime;
}

u64 memfnv1a(u64 base, ConstPtr data, usize size) {
	const ubyte *bytes = data;
	const ubyte *end = bytes + size;
	for (; bytes < end; bytes++) {
		base = (base ^ *bytes) * fnv1a_prime;
	}
	return base;
}
