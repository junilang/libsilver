#if defined(__x86_64__)
	#ifndef PTRTAG
		#define PTRTAG true
	#endif
	#define PTRTAG_BITS 16
	#define PTRTAG_ADDR_BITS 48
	typedef u16 utag;
	constexpr utag utag_max = u16_max;
	constexpr u8 utag_width = 16;
#else
	#if PTRTAG
		#error "pointer tagging unsupported by platform"
	#endif

	#define PTRTAG false
#endif

#ifndef PTRTAG_SAFE
	#define PTRTAG_SAFE BUILD_SAFE
#endif

#if PTRTAG
	#define PTRTAG_MAX ((1ull << PTRTAG_BITS) - 1)

	#if PTRTAG_LAM
		#error "PTRTAG_LAM unimplemented"
	#endif

	#if PTRTAG_CANONICAL
		#error "PTRTAG_CANONICAL unimplemented"
	#endif

	#define PTRTAG_LSB_MASK ((1ull << PTRTAG_BITS) - 1)
	#define PTRTAG_ADDR_MASK ((1ull << PTRTAG_ADDR_BITS) - 1)
	#define PTRTAG_MSB_MASK (~PTRTAG_ADDR_MASK)

	Ptr ptrtag_msb(ConstPtr ptr, utag tag) {
		#if PTRTAG_SAFE
			if ((usize)ptr & PTRTAG_MSB_MASK)
				PANIC("ptrtag_msb: pointer already tagged");

			if (tag > PTRTAG_MAX)
				PANIC("ptrtag_msb: tag overflow");
		#endif

		return (Ptr)(
			(usize)ptr | ((usize)tag << PTRTAG_ADDR_BITS)
		);
	}

	#define CONSTEXPR_ptrtag_msb(ptr, tag) \
		( (usize)(ptr) | ((usize)(tag) << PTRTAG_ADDR_BITS) )

	Ptr ptrstrip_msb(ConstPtr ptr) {
		return (Ptr)(
			(usize)ptr & PTRTAG_ADDR_MASK
		);
	}

	utag ptrread_msb(ConstPtr ptr) {
		return (utag)((usize)ptr >> PTRTAG_ADDR_BITS);
	}

	Ptr ptrtag_lsb(ConstPtr ptr, utag tag) {
		#if PTRTAG_SAFE
			if ((usize)ptr & (PTRTAG_MSB_MASK))
				PANIC("ptrtag_lsb: pointer already tagged");

			if (tag > PTRTAG_MAX)
				PANIC("ptrtag_msb: tag overflow");
		#endif

		return (Ptr)(
			((usize)ptr << PTRTAG_BITS) | (usize)tag
		);
	}

	#define CONSTEXPR_ptrtag_lsb(ptr, tag) \
		( ((usize)(ptr) << PTRTAG_BITS) | (usize)(tag) )

	Ptr ptrstrip_lsb(ConstPtr ptr) {
		return (Ptr)((usize)ptr >> PTRTAG_BITS);
	}

	utag ptrread_lsb(ConstPtr ptr) {
		return (utag)((usize)ptr & PTRTAG_LSB_MASK);
	}

	#if PTRTAG_MODE_LSB
		#define PTRTAG_DEFAULT(N) N##_lsb
		#define CONSTEXPR_ptrtag CONSTEXPR_ptrtag_lsb
	#else
		#define PTRTAG_DEFAULT(N) N##_msb
		#define CONSTEXPR_ptrtag CONSTEXPR_ptrtag_msb
	#endif

	Ptr ptrtag(ConstPtr ptr, utag tag) { return PTRTAG_DEFAULT(ptrtag)(ptr, tag); }
	Ptr ptrstrip(ConstPtr ptr) { return PTRTAG_DEFAULT(ptrstrip)(ptr); }
	utag ptrread(ConstPtr ptr) { return PTRTAG_DEFAULT(ptrread)(ptr); }

	#undef PTRTAG_DEFAULT

#endif
