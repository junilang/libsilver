#if !defined(HASH_USE_CRC64) && !defined(HASH_USE_CRC32) && !defined(HASH_USE_FNV1A)
	#if LIBSILVER_HAS_CRC32
		#define HASH_USE_CRC64 true
	#else
		#define HASH_USE_FNV1A true
	#endif
#endif


#if HASH_USE_CRC64
	#if !LIBSILVER_HAS_CRC32
		#error "crc64 not supported"
	#endif

	typedef u64 uhash;
	#define HASH_WIDTH 64
	constexpr u8 uhash_width = 64;
	constexpr uhash uhash_max = u64_max;
	constexpr uhash hash_base = crc64_base;

	uhash memhash(uhash base, ConstPtr data, usize size) {
		return memcrc64(base, data, size);
	}

	uhash u8_hash(uhash base, u8 value) {
		return u8_crc64(base, value);
	}

	uhash u16_hash(uhash base, u16 value) {
		return u16_crc64(base, value);
	}

	uhash u32_hash(uhash base, u32 value) {
		return u32_crc64(base, value);
	}

	uhash u64_hash(uhash base, u64 value) {
		return u64_crc64(base, value);
	}

#elif HASH_USE_CRC32
	#if !LIBSILVER_HAS_CRC32
		#error "crc32 not supported"
	#endif

	typedef u32 uhash;
	#define HASH_WIDTH 32
	constexpr u8 uhash_width = 32;
	constexpr uhash uhash_max = u32_max;
	constexpr uhash hash_base = crc32_base;

	uhash memhash(uhash base, ConstPtr data, usize size) {
		return memcrc32(base, data, size);
	}

	uhash u8_hash(uhash base, u8 value) {
		return u8_crc32(base, value);
	}

	uhash u16_hash(uhash base, u16 value) {
		return u16_crc32(base, value);
	}

	uhash u32_hash(uhash base, u32 value) {
		return u32_crc32(base, value);
	}

	uhash u64_hash(uhash base, u64 value) {
		return u64_crc32(base, value);
	}


#elif HASH_USE_FNV1A

	typedef u64 uhash;
	#define HASH_WIDTH 64
	constexpr u8 uhash_width = 64;
	constexpr uhash uhash_max = u64_max;
	constexpr uhash hash_base = fnv1a_base;

	uhash memhash(uhash base, ConstPtr data, usize size) {
		return memfnv1a(base, data, size);
	}

	uhash u8_hash(uhash base, u8 value) {
		return u8_fnv1a(base, value);
	}

	uhash u16_hash(uhash base, u16 value) {
		return u16_fnv1a(base, value);
	}

	uhash u32_hash(uhash base, u32 value) {
		return u32_fnv1a(base, value);
	}

	uhash u64_hash(uhash base, u64 value) {
		return u64_fnv1a(base, value);
	}

#endif
