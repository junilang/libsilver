#define CHAR_LOCKFREE (__GCC_ATOMIC_CHAR_LOCK_FREE == 2)
#define SHORT_LOCKFREE (__GCC_ATOMIC_SHORT_LOCK_FREE == 2)
#define INT_LOCKFREE (__GCC_ATOMIC_INT_LOCK_FREE == 2)
#define LONG_LOCKFREE (__GCC_ATOMIC_LONG_LOCK_FREE == 2)
#define LLONG_LOCKFREE (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)

#if CHAR_LOCKFREE
	#define CHAR_LOCKFREE_TYPE char
	#define CHAR_LOCKFREE_WIDTH CHAR_WIDTH
#elif SHORT_LOCKFREE
	#define CHAR_LOCKFREE_TYPE short
	#define CHAR_LOCKFREE_WIDTH SHORT_WIDTH
#elif INT_LOCKFREE
	#define CHAR_LOCKFREE_TYPE int
	#define CHAR_LOCKFREE_WIDTH INT_WIDTH
#elif LONG_LOCKFREE
	#define CHAR_LOCKFREE_TYPE long
	#define CHAR_LOCKFREE_WIDTH LONG_WIDTH
#elif LLONG_LOCKFREE
	#define CHAR_LOCKFREE_TYPE long long
	#define CHAR_LOCKFREE_WIDTH LLONG_WIDTH
#endif

#if SHORT_LOCKFREE
	#define SHORT_LOCKFREE_TYPE short
	#define SHORT_LOCKFREE_WIDTH SHORT_WIDTH
#elif INT_LOCKFREE
	#define SHORT_LOCKFREE_TYPE int
	#define SHORT_LOCKFREE_WIDTH INT_WIDTH
#elif LONG_LOCKFREE
	#define SHORT_LOCKFREE_TYPE long
	#define SHORT_LOCKFREE_WIDTH LONG_WIDTH
#elif LLONG_LOCKFREE
	#define SHORT_LOCKFREE_TYPE long long
	#define SHORT_LOCKFREE_WIDTH LLONG_WIDTH
#endif

#if INT_LOCKFREE
	#define INT_LOCKFREE_TYPE int
	#define INT_LOCKFREE_WIDTH INT_WIDTH
#elif LONG_LOCKFREE
	#define INT_LOCKFREE_TYPE long
	#define INT_LOCKFREE_WIDTH LONG_WIDTH
#elif LLONG_LOCKFREE
	#define INT_LOCKFREE_TYPE long long
	#define INT_LOCKFREE_WIDTH LLONG_WIDTH
#endif

#if LONG_LOCKFREE
	#define LONG_LOCKFREE_TYPE long
	#define LONG_LOCKFREE_WIDTH LONG_WIDTH
#elif LLONG_LOCKFREE
	#define LONG_LOCKFREE_TYPE long long
	#define LONG_LOCKFREE_WIDTH LLONG_WIDTH
#endif

#if LLONG_LOCKFREE
	#define LLONG_LOCKFREE_TYPE long long
	#define LLONG_LOCKFREE_WIDTH LLONG_WIDTH
#endif

#ifdef CHAR_LOCKFREE_TYPE
	typedef unsigned CHAR_LOCKFREE_TYPE uchar_lf;
	typedef signed CHAR_LOCKFREE_TYPE ichar_lf;

	constexpr u8 uchar_lf_width = CHAR_LOCKFREE_WIDTH;
	constexpr u8 ichar_lf_width = CHAR_LOCKFREE_WIDTH;

#endif

#ifdef SHORT_LOCKFREE_TYPE
	typedef unsigned SHORT_LOCKFREE_TYPE ushort_lf;
	typedef signed SHORT_LOCKFREE_TYPE ishort_lf;

	constexpr u8 ushort_lf_width = SHORT_LOCKFREE_WIDTH;
	constexpr u8 ishort_lf_width = SHORT_LOCKFREE_WIDTH;

#endif

#ifdef INT_LOCKFREE_TYPE
	typedef unsigned INT_LOCKFREE_TYPE uint_lf;
	typedef signed INT_LOCKFREE_TYPE int_lf;

	constexpr u8 uint_lf_width = INT_LOCKFREE_WIDTH;
	constexpr u8 int_lf_width = INT_LOCKFREE_WIDTH;

#endif

#ifdef LONG_LOCKFREE_TYPE
	typedef unsigned LONG_LOCKFREE_TYPE ulong_lf;
	typedef signed LONG_LOCKFREE_TYPE ilong_lf;

	constexpr u8 ulong_lf_width = LONG_LOCKFREE_WIDTH;
	constexpr u8 ilong_lf_width = LONG_LOCKFREE_WIDTH;

#endif

#ifdef LLONG_LOCKFREE_TYPE
	typedef unsigned LLONG_LOCKFREE_TYPE ullong_lf;
	typedef signed LLONG_LOCKFREE_TYPE illong_lf;

	constexpr u8 ullong_lf_width = LLONG_LOCKFREE_WIDTH;
	constexpr u8 illong_lf_width = LLONG_LOCKFREE_WIDTH;

#endif

#if CHAR_WIDTH == 8
	typedef uchar_lf u8_lf;
	typedef ichar_lf i8_lf;
	constexpr u8 u8_lf_width = uchar_lf_width;
	constexpr u8 i8_lf_width = ichar_lf_width;
	#define U8_LOCKFREE_WIDTH CHAR_LOCKFREE_WIDTH

#elif SHORT_WIDTH == 8
	typedef ushort_lf u8_lf;
	typedef ishort_lf i8_lf;
	constexpr u8 u8_lf_width = ushort_lf_width;
	constexpr u8 i8_lf_width = ishort_lf_width;
	#define U8_LOCKFREE_WIDTH SHORT_LOCKFREE_WIDTH

#elif INT_WIDTH == 8
	typedef uint_lf u8_lf;
	typedef int_lf i8_lf;
	constexpr u8 u8_lf_width = uint_lf_width;
	constexpr u8 i8_lf_width = int_lf_width;
	#define U8_LOCKFREE_WIDTH INT_LOCKFREE_WIDTH

#elif LONG_WIDTH == 8
	typedef ulong_lf u8_lf;
	typedef ilong_lf i8_lf;
	constexpr u8 u8_lf_width = ulong_lf_width;
	constexpr u8 i8_lf_width = ilong_lf_width;
	#define U8_LOCKFREE_WIDTH LONG_LOCKFREE_WIDTH

#elif LLONG_WIDTH == 8
	typedef ullong_lf u8_lf;
	typedef illong_lf i8_lf;
	constexpr u8 u8_lf_width = ullong_lf_width;
	constexpr u8 i8_lf_width = illong_lf_width;
	#define U8_LOCKFREE_WIDTH LLONG_LOCKFREE_WIDTH

#endif

#if CHAR_WIDTH == 16
	typedef uchar_lf u16_lf;
	typedef ichar_lf i16_lf;
	constexpr u16 u16_lf_width = uchar_lf_width;
	constexpr u16 i16_lf_width = ichar_lf_width;
	#define U16_LOCKFREE_WIDTH CHAR_LOCKFREE_WIDTH

#elif SHORT_WIDTH == 16
	typedef ushort_lf u16_lf;
	typedef ishort_lf i16_lf;
	constexpr u16 u16_lf_width = ushort_lf_width;
	constexpr u16 i16_lf_width = ishort_lf_width;
	#define U16_LOCKFREE_WIDTH SHORT_LOCKFREE_WIDTH

#elif INT_WIDTH == 16
	typedef uint_lf u16_lf;
	typedef int_lf i16_lf;
	constexpr u16 u16_lf_width = uint_lf_width;
	constexpr u16 i16_lf_width = int_lf_width;
	#define U16_LOCKFREE_WIDTH INT_LOCKFREE_WIDTH

#elif LONG_WIDTH == 16
	typedef ulong_lf u16_lf;
	typedef ilong_lf i16_lf;
	constexpr u16 u16_lf_width = ulong_lf_width;
	constexpr u16 i16_lf_width = ilong_lf_width;
	#define U16_LOCKFREE_WIDTH LONG_LOCKFREE_WIDTH

#elif LLONG_WIDTH == 16
	typedef ullong_lf u16_lf;
	typedef illong_lf i16_lf;
	constexpr u16 u16_lf_width = ullong_lf_width;
	constexpr u16 i16_lf_width = illong_lf_width;
	#define U16_LOCKFREE_WIDTH LLONG_LOCKFREE_WIDTH

#endif

#if CHAR_WIDTH == 32
	typedef uchar_lf u32_lf;
	typedef ichar_lf i32_lf;
	constexpr u32 u32_lf_width = uchar_lf_width;
	constexpr u32 i32_lf_width = ichar_lf_width;
	#define U32_LOCKFREE_WIDTH CHAR_LOCKFREE_WIDTH

#elif SHORT_WIDTH == 32
	typedef ushort_lf u32_lf;
	typedef ishort_lf i32_lf;
	constexpr u32 u32_lf_width = ushort_lf_width;
	constexpr u32 i32_lf_width = ishort_lf_width;
	#define U32_LOCKFREE_WIDTH SHORT_LOCKFREE_WIDTH

#elif INT_WIDTH == 32
	typedef uint_lf u32_lf;
	typedef int_lf i32_lf;
	constexpr u32 u32_lf_width = uint_lf_width;
	constexpr u32 i32_lf_width = int_lf_width;
	#define U32_LOCKFREE_WIDTH INT_LOCKFREE_WIDTH

#elif LONG_WIDTH == 32
	typedef ulong_lf u32_lf;
	typedef ilong_lf i32_lf;
	constexpr u32 u32_lf_width = ulong_lf_width;
	constexpr u32 i32_lf_width = ilong_lf_width;
	#define U32_LOCKFREE_WIDTH LONG_LOCKFREE_WIDTH

#elif LLONG_WIDTH == 32
	typedef ullong_lf u32_lf;
	typedef illong_lf i32_lf;
	constexpr u32 u32_lf_width = ullong_lf_width;
	constexpr u32 i32_lf_width = illong_lf_width;
	#define U32_LOCKFREE_WIDTH LLONG_LOCKFREE_WIDTH

#endif

#if CHAR_WIDTH == 64
	typedef uchar_lf u64_lf;
	typedef ichar_lf i64_lf;
	constexpr u64 u64_lf_width = uchar_lf_width;
	constexpr u64 i64_lf_width = ichar_lf_width;
	#define U64_LOCKFREE_WIDTH CHAR_LOCKFREE_WIDTH

#elif SHORT_WIDTH == 64
	typedef ushort_lf u64_lf;
	typedef ishort_lf i64_lf;
	constexpr u64 u64_lf_width = ushort_lf_width;
	constexpr u64 i64_lf_width = ishort_lf_width;
	#define U64_LOCKFREE_WIDTH SHORT_LOCKFREE_WIDTH

#elif INT_WIDTH == 64
	typedef uint_lf u64_lf;
	typedef int_lf i64_lf;
	constexpr u64 u64_lf_width = uint_lf_width;
	constexpr u64 i64_lf_width = int_lf_width;
	#define U64_LOCKFREE_WIDTH INT_LOCKFREE_WIDTH

#elif LONG_WIDTH == 64
	typedef ulong_lf u64_lf;
	typedef ilong_lf i64_lf;
	constexpr u64 u64_lf_width = ulong_lf_width;
	constexpr u64 i64_lf_width = ilong_lf_width;
	#define U64_LOCKFREE_WIDTH LONG_LOCKFREE_WIDTH

#elif LLONG_WIDTH == 64
	typedef ullong_lf u64_lf;
	typedef illong_lf i64_lf;
	constexpr u64 u64_lf_width = ullong_lf_width;
	constexpr u64 i64_lf_width = illong_lf_width;
	#define U64_LOCKFREE_WIDTH LLONG_LOCKFREE_WIDTH

#endif
