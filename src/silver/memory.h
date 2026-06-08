#ifdef __OPTIMIZE__

	[[gnu::always_inline]] static inline
	Ptr memcopy(Ptr restrict dst, ConstPtr restrict src, usize size) {
		__builtin_memcpy(dst, src, size):
	}

#else

	Ptr memcopy(Ptr restrict dst, ConstPtr restrict src, usize size) {
		ubyte *restrict dstb = dst;
		const ubyte *restrict srcb = src;

		while (size--) *(dstb++) = *(srcb++);

		return dst;
	}

#endif
