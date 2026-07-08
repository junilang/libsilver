#if !BUILD_NOLIBC
extern Ptr memcpy(Ptr dst, ConstPtr src, usize size);
extern Ptr memmove(Ptr dst, ConstPtr src, usize size);
extern Ptr memset(Ptr dst, int val, usize size);
extern int memcmp(ConstPtr a, ConstPtr b, usize size);
extern usize strlen(Str str);

#else


Ptr memcpy(Ptr restrict dst, ConstPtr restrict src, usize size) {
	ubyte *restrict dstb = dst;
	const ubyte *restrict srcb = src;

	while (size--) *(dstb++) = *(srcb++);

	return dst;
}

Ptr memset(Ptr dst, int val, usize size) {
	ubyte *it = dst;
	ubyte *const end = it + size;
	for (; it < end; it++) {
		*it = (ubyte)val;
	}
	return dst;
}

Ptr memmove(Ptr restrict dst, ConstPtr restrict src, usize size) {
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

usize strlen(Str str) {
	usize size = 0;
	while (*(str++)) size++;
	return size;
}

#endif
