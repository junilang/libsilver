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
#else
	#define CHAR_LOCKFREE_TYPE char
	#warning "no lockfree types suitable for char"
#endif

#if SHORT_LOCKFREE
	#define SHORT_LOCKFREE_TYPE short
#elif INT_LOCKFREE
	#define SHORT_LOCKFREE_TYPE int
#elif LONG_LOCKFREE
	#define SHORT_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define SHORT_LOCKFREE_TYPE long long
#else
	#define SHORT_LOCKFREE_TYPE short
	#warning "no lockfree types suitable for short"
#endif

#if INT_LOCKFREE
	#define INT_LOCKFREE_TYPE int
#elif LONG_LOCKFREE
	#define INT_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define INT_LOCKFREE_TYPE long long
#else
	#define INT_LOCKFREE_TYPE int
	#warning "no lockfree types suitable for int"
#endif

#if LONG_LOCKFREE
	#define LONG_LOCKFREE_TYPE long
#elif LLONG_LOCKFREE
	#define LONG_LOCKFREE_TYPE long long
#else
	#define LONG_LOCKFREE_TYPE long
	#warning "no lockfree types suitable for long"
#endif

#if LLONG_LOCKFREE
	#define LLONG_LOCKFREE_TYPE long long
#else
	#define LLONG_LOCKFREE_TYPE long long
	#warning "no lockfree types suitable for long long"
#endif

typedef unsigned CHAR_LOCKFREE_TYPE uchar_lf;
typedef signed CHAR_LOCKFREE_TYPE ichar_lf;

typedef unsigned SHORT_LOCKFREE_TYPE ushort_lf;
typedef signed SHORT_LOCKFREE_TYPE ishort_lf;

typedef unsigned INT_LOCKFREE_TYPE uint_lf;
typedef signed INT_LOCKFREE_TYPE int_lf;

typedef unsigned LONG_LOCKFREE_TYPE ulong_lf;
typedef signed LONG_LOCKFREE_TYPE ilong_lf;

typedef unsigned LLONG_LOCKFREE_TYPE ullong_lf;
typedef signed LLONG_LOCKFREE_TYPE illong_lf;
