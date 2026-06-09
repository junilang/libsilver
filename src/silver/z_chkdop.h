#define XUNSIGNED \
	X(uchar) \
	X(ushort) \
	X(uint) \
	X(ulong) \
	X(ullong)

#define XSIGNED \
	X(ichar) \
	X(ishort) \
	X(int) \
	X(ilong) \
	X(illong)

#if __has_builtin(add_overflow)
	#define chkdadd __builtin_add_overflow
#else

	// unsigned checked add
	#define X(T) \
		bool T##_chkdadd(T a, T b, T *result) { \
			if (a > (T##_max - b)) \
				return true; \
			*result = a + b; \
			return false; \
		}

		XUNSIGNED

	#undef X

	// signed checked add
	#define X(T) \
		bool T##_chkdadd(T a, T b, T *result) { \
			if ((b > 0) && (a > (T##_max - b))) \
				return true; \
			if (a < (T##_min - b)) \
				return true; \
			*result = a + b; \
			return false; \
		}

		XSIGNED

	#undef X

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

#endif

#define X(T) \
	bool T##_chkdsub(T a, T b, T *result) { \
		if (b > a) \
			return true; \
		*result = a - b; \
		return false; \
	}

	XUNSIGNED

#undef X

#define X(T) \
	bool T##_chkdsub(T a, T b, T *result) { \
	}

#undef X

#undef XUNSIGNED
#undef XSIGNED
