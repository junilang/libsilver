#if !defined(HASH_USE_CRC64) && !defined(HASH_USE_CRC32) && !defined(HASH_USE_FNV1A)
	#if LIBSILVER_HAS_CRC32
		#define HASH_USE_CRC64 true
	#else
		#define HASH_USE_FNV1A true
	#endif
#endif

#if PLATFORM_LP64
	#define HASH(base, value) _Generic((value), \
		signed char : i8_hash, \
		signed short : i16_hash, \
		signed int : i32_hash, \
		signed long : i64_hash, \
		signed long long : i64_hash, \
		unsigned char : u8_hash, \
		unsigned short : u16_hash, \
		unsigned int : u32_hash, \
		unsigned long : u64_hash, \
		unsigned long long : u64_hash, \
		Ptr : Ptr_hash, \
		ConstPtr : Ptr_hash \
	)(base, (value))

#elif PLATFORM_LLP64
	#define HASH(base, value) _Generic((value), \
		signed char : i8_hash, \
		signed short : i16_hash, \
		signed int : i32_hash, \
		signed long : i32_hash, \
		signed long long : i64_hash, \
		unsigned char : u8_hash, \
		unsigned short : u16_hash, \
		unsigned int : u32_hash, \
		unsigned long : u32_hash, \
		unsigned long long : u64_hash, \
		Ptr : Ptr_hash, \
		ConstPtr : Ptr_hash \
	)(base, (value))

#else
	#error "unsupported platform"
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

uhash i8_hash(uhash base, i8 value) {
	return u8_hash(base, (u8)value);
}

uhash i16_hash(uhash base, i16 value) {
	return u16_hash(base, (u16)value);
}

uhash i32_hash(uhash base, i32 value) {
	return u32_hash(base, (u32)value);
}

uhash i64_hash(uhash base, i64 value) {
	return u64_hash(base, (u64)value);
}

uhash Ptr_hash(uhash base, ConstPtr value) {
	return u64_hash(base, (u64)value);
}
