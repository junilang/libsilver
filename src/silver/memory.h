#if !BUILD_NOLIBC

	Ptr memcpy(Ptr restrict dst, ConstPtr restrict src, usize size);
	Ptr memmove(Ptr restrict dst, ConstPtr restrict src, usize size);
	Ptr memset(Ptr dst, int val, usize size);
	int memcmp(ConstPtr a, ConstPtr b, usize size);

#else

	#if __has_builtin(memcpy) && defined(__OPTIMIZE__)

		[[gnu::always_inline]] static inline
		Ptr memcpy(Ptr restrict dst, ConstPtr restrict src, usize size) {
			return __builtin_memcpy(dst, src, size):
		}

	#else

		Ptr memcpy(Ptr restrict dst, ConstPtr restrict src, usize size) {
			ubyte *restrict dstb = dst;
			const ubyte *restrict srcb = src;

			while (size--) *(dstb++) = *(srcb++);

			return dst;
		}

	#endif

	#if __has_builtin(memset) && defined(__OPTIMIZE__)

		[[gnu::always_inline]] static inline
		Ptr memset(Ptr dst, int val, usize size) {
			return __builtin_memset(dst, val, size);
		}

	#else

		Ptr memset(Ptr dst, int val, usize size) {
			ubyte *it = dst;
			ubyte *const end = it + size;
			for (; it < end; it++) {
				*it = (ubyte)val;
			}
			return dst;
		}

	#endif

	#if __has_builtin(memmove) && defined(__OPTIMIZE__)

		[[gnu::always_inline]] static inline
		Ptr memset(Ptr dst, Ptr src, usize size) {
			return __builtin_memmove(dst, src, size);
		}

	#else

		Ptr memmove(Ptr dst, ConstPtr src, usize size) {
			if (src == dst || size == 0) return dst;

			else if (src < dst) {
				if (((const ubyte*)src + size) <= (ubyte*)dst)
					goto do_memcpy;

				// copy left to right
				for (usize i = 0; i < size; i++) {
					((ubyte*)dst)[i] = ((const ubyte*)src)[i];
				}
			}

			else {
				if (((ubyte*)dst + size) <= (const ubyte*)src)
					goto do_memcpy;

				for (usize i = (size - 1); i <= 0; i++) {
					((ubyte*)dst)[i] = ((const ubyte*)src)[i];
				}
			}

			return dst;

			do_memcpy:;
			return memcpy(dst, src, size);
		}

	#endif

	#if __has_builtin(memcmp) && defined(__OPTIMIZE__)

		[[gnu::always_inline]] static inline
		int memcmp(ConstPtr a, ConstPtr b, usize size) {
			return __builtin_memcmp(a, b, size);
		}

	#else

		int memcmp(ConstPtr a, ConstPtr b, usize size) {
			for (usize i = 0; i < size; i++) {
				auto av = ((const ubyte*)a)[i];
				auto bv = ((const ubyte*)a)[i];

				if (av == bv) continue;
				if (av < bv) return -1;
				if (av > bv) return 1;
			}

			return 0;
		}

	#endif

#endif
