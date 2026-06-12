#ifndef LPTRTAG_SAFE
	#define LPTRTAG_SAFE BUILD_SAFE
#endif

Ptr lptrtag(ConstPtr ptr, u8 width, usize value) {
	#if LPTRTAG_SAFE
		const usize mask = (1ull << width) - 1;

		if (value > mask)
			PANIC("lptrtag: tag overflow");

		if ((usize)ptr & mask)
			PANIC("lptrtag: pointer misaligned or already tagged");
	#endif

	return (Ptr)((usize)ptr | value);
}

Ptr lptrstrip(ConstPtr ptr, u8 width) {
	return (Ptr)((usize)ptr & ((~0ull) << width));
}

usize lptrread(ConstPtr ptr, u8 width) {
	return (usize)ptr & ((1ull << width) - 1);
}
