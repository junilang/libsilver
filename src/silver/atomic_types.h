#define CHAR_LOCKFREE (__GCC_ATOMIC_CHAR_LOCK_FREE == 2)
#define SHORT_LOCKFREE (__GCC_ATOMIC_SHORT_LOCK_FREE == 2)
#define INT_LOCKFREE (__GCC_ATOMIC_INT_LOCK_FREE == 2)
#define LONG_LOCKFREE (__GCC_ATOMIC_LONG_LOCK_FREE == 2)
#define LLONG_LOCKFREE (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)

#if CHAR_LOCKFREE
	#define CHAR_LOCKFREE_TYPE char
#elif SHORT_LOCKFREE
	#define CHAR_LOCKFREE_TYPE short
#elif INT_LOCKFREE
	#define CHAR_LOCKFREE_TYPE int
#elif LONG_LOCKFREE
	#define CHAR_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define CHAR_LOCKFREE_TYPE long long
#endif

#if SHORT_LOCKFREE
	#define SHORT_LOCKFREE_TYPE short
#elif INT_LOCKFREE
	#define SHORT_LOCKFREE_TYPE int
#elif LONG_LOCKFREE
	#define SHORT_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define SHORT_LOCKFREE_TYPE long long
#endif

#if INT_LOCKFREE
	#define INT_LOCKFREE_TYPE int
#elif LONG_LOCKFREE
	#define INT_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define INT_LOCKFREE_TYPE long long
#endif

#if LONG_LOCKFREE
	#define LONG_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define LONG_LOCKFREE_TYPE long long
#endif

#if 0 //LLONG_LOCKFREE
	#define LLONG_LOCKFREE_TYPE long long
#endif

#ifdef CHAR_LOCKFREE_TYPE
	typedef unsigned CHAR_LOCKFREE_TYPE uchar_lf;
	typedef signed CHAR_LOCKFREE_TYPE ichar_lf;

#else
	[[deprecated("not lock free")]] typedef unsigned char uchar_lf;
	[[deprecated("not lock free")]] typedef signed char uchar_lf;

#endif

#ifdef SHORT_LOCKFREE_TYPE
	typedef unsigned SHORT_LOCKFREE_TYPE ushort_lf;
	typedef signed SHORT_LOCKFREE_TYPE ishort_lf;

#else
	[[deprecated("not lock free")]] typedef unsigned short ushort_lf;
	[[deprecated("not lock free")]] typedef signed short ishort_lf;

#endif

#ifdef INT_LOCKFREE_TYPE
	typedef unsigned INT_LOCKFREE_TYPE uint_lf;
	typedef signed INT_LOCKFREE_TYPE int_lf;

#else
	[[deprecated("not lock free")]] typedef unsigned int uint_lf;
	[[deprecated("not lock free")]] typedef signed int int_lf;

#endif

#ifdef LONG_LOCKFREE_TYPE
	typedef unsigned LONG_LOCKFREE_TYPE ulong_lf;
	typedef signed LONG_LOCKFREE_TYPE ilong_lf;

#else
	[[deprecated("not lock free")]] typedef unsigned long ulong_lf;
	[[deprecated("not lock free")]] typedef signed long ilong_lf;

#endif

#ifdef LLONG_LOCKFREE_TYPE
	typedef unsigned LLONG_LOCKFREE_TYPE ullong_lf;
	typedef signed LLONG_LOCKFREE_TYPE illong_lf;

#else
	[[deprecated("not lock free")]] typedef unsigned long long ullong_lf;
	[[deprecated("not lock free")]] typedef signed long long illong_lf;

#endif
