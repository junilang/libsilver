#define XTYPES \
	XU(uchar) \
	XU(ushort) \
	XU(uint) \
	XU(ulong) \
	XU(ullong) \
	XS(ichar) \
	XS(ishort) \
	XS(int) \
	XS(ilong) \
	XS(illong)

#if __has_builtin(__builtin_add_overflow)
	#define XU(T) \
		[[gnu::always_inline]] static inline \
		bool T##_chkdadd(T a, T b, T *result) { \
			return __builtin_add_overflow(a, b, result); \
		}

	#define XS XU

#else
	#define XU(T) \
		bool T##_chkdadd(T a, T b, T *result) { \
			if (a > (T##_max - b)) \
				return true; \
			*result = a + b; \
			return false; \
		}

	#define XS(T) \
		bool T##_chkdadd(T a, T b, T *result) { \
			if ((b > 0) && (a > (T##_max - b))) \
				return true; \
			if (a < (T##_min - b)) \
				return true; \
			*result = a + b; \
			return false; \
		}

#endif

XTYPES

#undef XU
#undef XS

#define chkdadd(a, b, result) _Generic((*(result)), \
	uchar : uchar_chkdadd, \
	ushort : ushort_chkdadd, \
	uint : uint_chkdadd, \
	ulong : ulong_chkdadd, \
	ullong : ullong_chkdadd, \
	ichar : ichar_chkdadd, \
	ishort : ishort_chkdadd, \
	int : int_chkdadd, \
	ilong : ilong_chkdadd, \
	illong : illong_chkdadd \
)((a), (b), (result))

#if __has_builtin(__builtin_sub_overflow)
	#define XU(T) \
		[[gnu::always_inline]] static inline \
		bool T##_chkdsub(T a, T b, T *result) { \
			return __builtin_sub_overflow(a, b, result); \
		}

	#define XS XU

#else
	#define XU(T) \
		bool T##_chkdsub(T a, T b, T *result) { \
			if (b > a) \
				return true; \
			*result = a - b; \
			return false; \
		}

	// TODO
	#error "chkdsub unimplemented"
	#define XS(T)

#endif

XTYPES

#undef XU
#undef XS

#define chkdsub(a, b, result) _Generic((*(result)), \
	uchar : uchar_chkdsub, \
	ushort : ushort_chkdsub, \
	uint : uint_chkdsub, \
	ulong : ulong_chkdsub, \
	ullong : ullong_chkdsub, \
	ichar : ichar_chkdsub, \
	ishort : ishort_chkdsub, \
	int : int_chkdsub, \
	ilong : ilong_chkdsub, \
	illong : illong_chkdsub \
)((a), (b), (result))

#if __has_builtin(__builtin_mul_overflow)
	#define XU(T) \
		[[gnu::always_inline]] static inline \
		bool T##_chkdmul(T a, T b, T *result) { \
			return __builtin_mul_overflow(a, b, result); \
		}

	#define XS XU
#else
	#error "chkdmul unimplemented"

	#define XU(T)
	#define XS(T)

#endif

XTYPES

#define chkdmul(a, b, result) _Generic((*(result)), \
	uchar : uchar_chkdsub, \
	ushort : ushort_chkdsub, \
	uint : uint_chkdsub, \
	ulong : ulong_chkdsub, \
	ullong : ullong_chkdsub, \
	ichar : ichar_chkdsub, \
	ishort : ishort_chkdsub, \
	int : int_chkdsub, \
	ilong : ilong_chkdsub, \
	illong : illong_chkdsub \
)((a), (b), (result))

#undef XU
#undef XS

#undef XTYPES

bool usize_chkdflx(usize base, usize elem, usize n, usize *result) {
	return chkdmul(elem, n, &elem) || chkdadd(base, elem, result);
}
