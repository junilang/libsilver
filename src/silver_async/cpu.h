#if __linux__
	#define CPU_SCHED_YIELD linux_sched_yield()

#else
	#warning "CPU_SCHED_YIELD undefined"

#endif

#if __x86_64__
	#define CPU_YIELD __builtin_ia32_pause()
	#define CPU_LO_STRONG true
	#define CPU_LO_WEAK false

	#ifndef CPU_HDI_SIZE
		#define CPU_HDI_SIZE 128
	#endif

	constexpr ualign cpu_hdi_size = CPU_HDI_SIZE;

#else
	#error "unsupported architecture"

#endif
