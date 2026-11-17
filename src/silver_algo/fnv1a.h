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

u64 i8_fnv1a(u64 base, i8 value) { return u8_fnv1a(base, (u8)value); }
u64 i16_fnv1a(u64 base, i16 value) { return u16_fnv1a(base, (u16)value); }
u64 i32_fnv1a(u64 base, i32 value) { return u32_fnv1a(base, (u32)value); }
u64 i64_fnv1a(u64 base, i64 value) { return u64_fnv1a(base, (u64)value); }

u64 fnv1a(u64 base, ConstPtr data, usize size) {
	const ubyte *bytes = data;
	const ubyte *end = bytes + size;
	for (; bytes < end; bytes++) {
		base = (base ^ *bytes) * fnv1a_prime;
	}
	return base;
}
