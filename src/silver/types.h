typedef __UINT8_TYPE__ u8;
constexpr u8 u8_max = (u8)(~0ull);

typedef __UINT16_TYPE__ u16;
constexpr u16 u16_max = (u16)(~0ull);

typedef __UINT32_TYPE__ u32;
constexpr u32 u32_max = (u32)(~0ull);

typedef __UINT64_TYPE__ u64;
constexpr u64 u64_max = (u64)(~0ull);

typedef __INT8_TYPE__ i8;
constexpr i8 i8_max = (i8)(u8_max >> 1);
constexpr i8 i8_min = -i8_max - 1;

typedef __INT16_TYPE__ i16;
constexpr i16 i16_max = (i16)(u16_max >> 1);
constexpr i16 i16_min = -i16_max - 1;

typedef __INT32_TYPE__ i32;
constexpr i32 i32_max = (u32)(u32_max >> 1);
constexpr i32 i32_min =	-i32_max - 1;

typedef __INT64_TYPE__ i64;
constexpr i64 i64_max = (i64)(u64_max >> 1);
constexpr i64 i64_min = -i64_max - 1;

typedef unsigned char ubyte;
constexpr ubyte ubyte_max = (ubyte)(~0ull);
#define BYTE_WIDTH __CHAR_BIT__
constexpr u8 ubyte_width = BYTE_WIDTH;

typedef signed char ibyte;
constexpr ibyte ibyte_max = (ibyte)(ubyte_max >> 1);
constexpr ibyte ibyte_min = -ibyte_max - 1;
constexpr u8 ibyte_width = BYTE_WIDTH;

typedef void *Ptr;
typedef const void *ConstPtr;
typedef const char *Str;

#if __linux__ && __x86_64__
	// LP64
	typedef unsigned long uword;
	typedef signed long iword;
	#define WORD_WIDTH 64

#else
	#error "unsupported platform"

#endif

constexpr uword uword_max = (uword)(~0ull);
constexpr u8 uword_width = WORD_WIDTH;

constexpr iword iword_max = (iword)(uword_max >> 1);
constexpr iword iword_min = -iword_max - 1;
constexpr u8 iword_width = uword_width;

static_assert(sizeof(uword) == sizeof(Ptr));
static_assert(sizeof(iword) == sizeof(Ptr));

#ifndef SIZE_WIDTH
	#define SIZE_WIDTH WORD_WIDTH
#elif SIZE_WIDTH != WORD_WIDTH
	#error
#endif

typedef uword usize;
constexpr usize usize_max = (usize)(~0ull);
constexpr u8 usize_width = uword_width;

typedef iword isize;
constexpr isize isize_max = iword_max;
constexpr isize isize_min = iword_min;
constexpr u8 isize_width = iword_width;

typedef u16 ualign;
constexpr ualign ualign_max = u16_max;
constexpr u8 ualign_width = 16;
#define UALIGN_WIDTH 16

// builtin standard c types
typedef unsigned char uchar;
constexpr uchar uchar_max = (uchar)(~0ull);
constexpr u8 uchar_width = __CHAR_BIT__;
#define CHAR_WIDTH __CHAR_BIT__

typedef signed char ichar;
constexpr ichar ichar_max = (char)(uchar_max >> 1);
constexpr ichar ichar_min = -ichar_max - 1;
constexpr u8 ichar_width = uchar_width;

typedef unsigned short ushort;
constexpr ushort ushort_max = (ushort)(~0ull);
constexpr u8 ushort_width = __SHRT_WIDTH__;
#define SHORT_WIDTH __SHRT_WIDTH__

typedef signed short ishort;
constexpr ishort ishort_max = (ishort)(ushort_max >> 1);
constexpr ishort ishort_min = -ishort_max - 1;
constexpr u8 ishort_width = ushort_width;

typedef unsigned int uint;
constexpr uint uint_max = (uint)(~0ull);
constexpr u8 int_width = __INT_WIDTH__;
#define INT_WIDTH __INT_WIDTH__

constexpr int int_max = (int)(uint_max >> 1);
constexpr int int_min = -int_max - 1;
constexpr u8 uint_width = int_width;

typedef unsigned long ulong;
constexpr ulong ulong_max = (ulong)(~0ull);
constexpr u8 ulong_width = __LONG_WIDTH__;
#define LONG_WIDTH __LONG_WIDTH__

typedef signed long ilong;
constexpr ilong ilong_max = (ilong)(ulong_max >> 1);
constexpr ilong ilong_min = -ilong_max - 1;
constexpr u8 ilong_width = ulong_width;

#ifdef __clang__
	#define LLONG_WIDTH __LLONG_WIDTH__
#else
	#define LLONG_WIDTH __LONG_LONG_WIDTH__
#endif

typedef unsigned long long ullong;
constexpr ullong ullong_max = (ullong)(~0ull);
constexpr u8 ullong_width = LLONG_WIDTH;

typedef signed long long illong;
constexpr illong illong_max = (illong)(ullong_max >> 1);
constexpr illong illong_min = -illong_max - 1;
constexpr u8 illong_width = ullong_width;
