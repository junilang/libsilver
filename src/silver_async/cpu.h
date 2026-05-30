#define CPU_YIELD sched_yield()

#if defined(__x86_64__)
	#define CPU_PAUSE __builtin_ia32_pause()
	#define CPU_TIME __rdtsc()
#else
	#define CPU_PAUSE
	#define CPU_TIME clock_gettime()
#endif
