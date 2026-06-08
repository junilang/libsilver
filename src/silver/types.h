typedef __u8 u8;
constexpr u8 u8_max = (u8)(~0ull);

typedef __u16 u16;
constexpr u16 u16_max = (u16)(~0ull);

typedef __u32 u32;
constexpr u32 u32_max = (u32)(~0ull);

typedef __u64 u64;
constexpr u64 u64_max = (u64)(~0ull);

typedef __s8 i8;
constexpr i8 i8_max = (i8)(u8_max >> 1);
constexpr i8 i8_min = -i8_max - 1;

typedef __s16 i16;
constexpr i16 i16_max = (i16)(u16_max >> 1);
constexpr i16 i16_min = -i16_max - 1;

typedef __s32 i32;
constexpr i32 i32_max = (u32)(u32_max >> 1);
constexpr i32 i32_min =	-i32_max - 1;

typedef __s64 i64;
constexpr i64 i64_max = (i64)(u64_max >> 1);
constexpr i64 i64_min = -i64_max - 1;

typedef __u8 ubyte;
constexpr ubyte ubyte_max = u8_max;

typedef __s8 ibyte;
constexpr ibyte ibyte_max = i8_max;
constexpr ibyte ibyte_min = i8_min;

typedef __kernel_size_t usize;
typedef __kernel_ssize_t isize;

static_assert(sizeof(usize) == sizeof(isize));

constexpr usize usize_max = (usize)(~0ull);
constexpr isize isize_max = (isize)(usize_max >> 1);
constexpr isize isize_min = -isize_max - 1;

typedef void *Ptr;
typedef const void *ConstPtr;
typedef const char *Str;

static_assert(sizeof(Ptr) == sizeof(usize));

typedef u16 ualign;
constexpr ualign ualign_max = u16_max;
constexpr u8 ualign_width = 16;

typedef unsigned char uchar;
constexpr uchar uchar_max = (uchar)(~0ull);

typedef signed char ichar;
constexpr ichar ichar_max = (char)(uchar_max >> 1);
constexpr ichar ichar_min = -ichar_max - 1;

typedef unsigned short ushort;
constexpr ushort ushort_max = (ushort)(~0ull);

typedef signed short ishort;
constexpr ishort ishort_max = (ishort)(ushort_max >> 1);
constexpr ishort ishort_min = -ishort_max - 1;

typedef unsigned int uint;
constexpr uint uint_max = (uint)(~0ull);

constexpr int int_max = (int)(uint_max >> 1);
constexpr int int_min = -int_max - 1;

typedef unsigned long ulong;
constexpr ulong ulong_max = (ulong)(~0ull);

typedef signed long ilong;
constexpr ilong ilong_max = (ilong)(ulong_max >> 1);
constexpr ilong ilong_min = -ilong_max - 1;

typedef unsigned long long ullong;

constexpr ullong ullong_max = (ullong)(~0ull);

typedef signed long long illong;

constexpr illong illong_max = (illong)(ullong_max >> 1);
constexpr illong illong_min = -illong_max - 1;

// WIDTH CALCULATIONS

#define X_UNSIGNED_WIDTH(T) \
	constexpr u8 T##_width = ( \
		T##_max == u8_max ? 8 : ( \
			T##_max == u16_max ? 16 : ( \
				T##_max == u32_max ? 32 : ( \
					T##_max == u64_max ? 64 : 0 \
				) \
			) \
		) \
	); \
	static_assert(T##_width != 0);

X_UNSIGNED_WIDTH(usize)
X_UNSIGNED_WIDTH(uchar)
X_UNSIGNED_WIDTH(ushort)
X_UNSIGNED_WIDTH(uint)
X_UNSIGNED_WIDTH(ulong)
X_UNSIGNED_WIDTH(ullong)


#undef XUNSIGNED_WIDTH

#define X_SIGNED_WIDTH(T) \
	constexpr T T##_width = ( \
		T##_max == i8_max ? 8 : ( \
			T##_max == i16_max ? 16 : ( \
				T##_max == i32_max ? 32 : ( \
					T##_max == i64_max ? 64 : 0 \
				) \
			) \
		) \
	); \
	static_assert(T##_width != 0);

X_SIGNED_WIDTH(isize)
X_SIGNED_WIDTH(ichar)
X_SIGNED_WIDTH(ishort)
X_SIGNED_WIDTH(int)
X_SIGNED_WIDTH(ilong)
X_SIGNED_WIDTH(illong)

#undef XSIGNED_WIDTH
