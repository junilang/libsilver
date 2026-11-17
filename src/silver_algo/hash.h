#if PLATFORM_LP64
	#define HASH(algo, base, value) _Generic((value), \
		signed char : i8_##algo, \
		signed short : i16_##algo, \
		signed int : i32_##algo, \
		signed long : i64_##algo, \
		signed long long : i64_##algo, \
		unsigned char : u8_##algo, \
		unsigned short : u16_##algo, \
		unsigned int : u32_##algo, \
		unsigned long : u64_##algo, \
		unsigned long long : u64_##algo \
	)(base, (value))

#elif PLATFORM_LLP64
	#define HASH(algo, base, value) _Generic((value), \
		signed char : i8_##algo, \
		signed short : i16_##algo, \
		signed int : i32_##algo, \
		signed long : i32_##algo, \
		signed long long : i64_##algo, \
		unsigned char : u8_##algo, \
		unsigned short : u16_##algo, \
		unsigned int : u32_##algo, \
		unsigned long : u32_##algo, \
		unsigned long long : u64_##algo \
	)(base, (value))

#else
	#error "unsupported platform"
#endif
