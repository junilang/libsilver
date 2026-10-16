#if __x86_64__
	#define CPU_YIELD __builtin_ia32_pause()
	#define CPU_LO_STRONG true
	#define CPU_LO_WEAK false

	#ifndef CPU_HDI_SIZE
		#define CPU_HDI_SIZE 64
	#endif

	#ifndef CPU_HCI_SIZE
		#define CPU_HCI_SIZE 64
	#endif

	constexpr usize cpu_hdi_size = CPU_HDI_SIZE;
	constexpr usize cpu_hci_size = CPU_HCI_SIZE;

	// endianness
	#define CPU_LE true
	#define CPU_BE false

#else
	#error "unsupported architecture"

#endif
