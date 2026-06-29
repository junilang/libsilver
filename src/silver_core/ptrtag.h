#if defined(__x86_64__)
	#ifndef PTRTAG
		#define PTRTAG true
	#endif
	#define PTRTAG_WIDTH 16
	#define PTRTAG_ADDR_WIDTH 48
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
	#if PTRTAG_LAM
		#error "PTRTAG_LAM unimplemented"
	#endif

	#if PTRTAG_CANONICAL
		#error "PTRTAG_CANONICAL unimplemented"
	#endif

	#if PTRTAG_WIDTH <= 8
		#define PTRTAG_CAN_OVERFLOW (PTRTAG_WIDTH < 8)
		typedef u8 utag;
	#elif PTRTAG_WIDTH <= 16
		#define PTRTAG_CAN_OVERFLOW (PTRTAG_WIDTH < 16)
		typedef u16 utag;
	#elif PTRTAG_WIDTH <= 32
		#define PTRTAG_CAN_OVERFLOW (PTRTAG_WIDTH < 32)
		typedef u32 utag;
	#endif

	#define PTRTAG_MAX ((1ull << PTRTAG_WIDTH) - 1)

	constexpr utag utag_max = PTRTAG_MAX;
	constexpr u8 utag_width = PTRTAG_WIDTH;

	constexpr u8 ptrtag_addr_width = PTRTAG_ADDR_WIDTH;
	constexpr usize ptrtag_lsb_mask = (1ull << utag_width) - 1;
	constexpr usize ptrtag_addr_mask = (1ull << ptrtag_addr_width) - 1;
	constexpr usize ptrtag_msb_mask = ~ptrtag_addr_mask;

	Ptr ptrtag_msb(ConstPtr ptr, utag tag) {
		#if PTRTAG_SAFE
			if ((usize)ptr & ptrtag_msb_mask)
				PANIC("ptrtag_msb: pointer already tagged");

			#if PTRTAG_CAN_OVERFLOW
				if (tag > utag_max)
					PANIC("ptrtag_msb: tag overflow");
			#endif
		#endif

		return (Ptr)(
			(usize)ptr | ((usize)tag << ptrtag_addr_width)
		);
	}

	#define CONSTEXPR_ptrtag_msb(ptr, tag) \
		( (usize)(ptr) | ((usize)(tag) << ptrtag_addr_width) )

	Ptr ptrstrip_msb(ConstPtr ptr) {
		return (Ptr)(
			(usize)ptr & ptrtag_addr_mask
		);
	}

	utag ptrread_msb(ConstPtr ptr) {
		return (utag)((usize)ptr >> ptrtag_addr_width);
	}

	Ptr ptrtag_lsb(ConstPtr ptr, utag tag) {
		#if PTRTAG_SAFE
			if ((usize)ptr & ptrtag_msb_mask)
				PANIC("ptrtag_lsb: pointer already tagged");

			#if PTRTAG_CAN_OVERFLOW
				if (tag > utag_max)
					PANIC("ptrtag_lsb: tag overflow");
			#endif
		#endif

		return (Ptr)(
			((usize)ptr << utag_width) | (usize)tag
		);
	}

	#define CONSTEXPR_ptrtag_lsb(ptr, tag) \
		( ((usize)(ptr) << utag_width) | (usize)(tag) )

	Ptr ptrstrip_lsb(ConstPtr ptr) {
		return (Ptr)((usize)ptr >> utag_width);
	}

	utag ptrread_lsb(ConstPtr ptr) {
		return (utag)((usize)ptr & ptrtag_lsb_mask);
	}

	#if PTRTAG_MODE_LSB
		#define XPTRTAG_DEFAULT(N) N##_lsb
		#define CONSTEXPR_ptrtag CONSTEXPR_ptrtag_lsb
	#else
		#define XPTRTAG_DEFAULT(N) N##_msb
		#define CONSTEXPR_ptrtag CONSTEXPR_ptrtag_msb
	#endif

	Ptr ptrtag(ConstPtr ptr, utag tag) { return XPTRTAG_DEFAULT(ptrtag)(ptr, tag); }
	Ptr ptrstrip(ConstPtr ptr) { return XPTRTAG_DEFAULT(ptrstrip)(ptr); }
	utag ptrread(ConstPtr ptr) { return XPTRTAG_DEFAULT(ptrread)(ptr); }

	#undef XPTRTAG_DEFAULT

#endif
