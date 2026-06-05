#if defined(__x86_64__)
	#define CPU_YIELD __builtin_ia32_pause()

#else
	#define CPU_YIELD

#endif

// detect target CPU default load ordering
#if defined(__x86_64__)
	#define CPU_LO_STRONG true
	#define CPU_LO_WEAK false

	#ifndef CPU_HDI_SIZE
		#define CPU_HDI_SIZE 128
	#endif

	#ifndef CPU_HDI_SIZE_CONSERVATIVE
		#define CPU_HDI_SIZE_CONSERVATIVE 64
	#endif

#else
	#error "architecture not handled"

#endif
