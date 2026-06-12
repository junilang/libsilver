#if __has_builtin(__builtin_ia32_crc32di)
#define LIBSILVER_HAS_CRC32 true

constexpr u32 crc32_base = 0x12345678ull;

u32 u8_crc32(u32 base, u8 value) {
	return __builtin_ia32_crc32qi(base, value);
}

u32 u16_crc32(u32 base, u16 value) {
	return __builtin_ia32_crc32hi(base, value);
}

u32 u32_crc32(u32 base, u32 value) {
	return __builtin_ia32_crc32si(base, value);
}

u32 u64_crc32(u32 base, u64 value) {
	return (u32)__builtin_ia32_crc32di(base, value);
}

u32 memcrc32(u32 base, ConstPtr data, usize size) {
	const ubyte *bytes = data;
	const ubyte *end = bytes + size;

	if (size < 8) goto rest;

	ualign align;
	if ((align = (usize)bytes & 0b1111)) {
		for (ualign i = 0; i < align; i++) {
			base = u8_crc32(base, bytes[i]);
		}
		bytes += align;
	}

	// aligned to 8 bytes
	while ((end - bytes) >= 8) {
		base = (u32)u64_crc32(base, *(u64*)bytes);
		bytes += 8;
	}

	rest:;
	while (bytes < end) {
		base = u8_crc32(base, *bytes);
		bytes++;
	}

	return base;
}

constexpr u64 crc64_base = 0x0123456789abcdefull;

u64 u8_crc64(u64 base, u8 value) {
	return
		((u64)__builtin_ia32_crc32qi((u32)(base >> 32), value) << 32)
		| __builtin_ia32_crc32qi((u32)base, value)
	;
}

u64 u16_crc64(u64 base, u16 value) {
	return
		((u64)__builtin_ia32_crc32hi((u32)(base >> 32), value) << 32)
		| __builtin_ia32_crc32hi((u32)base, value)
	;
}

u64 u32_crc64(u64 base, u32 value) {
	return
		((u64)__builtin_ia32_crc32si((u32)(base >> 32), value) << 32)
		| __builtin_ia32_crc32si((u32)base, value)
	;
}

u64 u64_crc64(u64 base, u64 value) {
	return
		((u64)__builtin_ia32_crc32di((u32)(base >> 32), value) << 32)
		| __builtin_ia32_crc32di((u32)base, value)
	;
}

u64 memcrc64(u64 base, ConstPtr data, usize size) {
	const ubyte *bytes = data;
	const ubyte *end = bytes + size;

	if (size < 8) goto rest;

	ualign align;
	if ((align = (usize)bytes & 0b1111)) {
		for (ualign i = 0; i < align; i++) {
			base = u8_crc64(base, bytes[i]);
		}
		bytes += align;
	}

	// aligned to 8 bytes
	while ((end - bytes) >= 8) {
		base = (u32)u64_crc64(base, *(u64*)bytes);
		bytes += 8;
	}

	rest:;
	while (bytes < end) {
		base = u8_crc64(base, *bytes);
		bytes++;
	}

	return base;
}

#else
#define LIBSILVER_HAS_CRC32 false

#endif
