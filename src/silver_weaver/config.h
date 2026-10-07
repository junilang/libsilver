#ifndef Weaver_SAFE
	#define Weaver_SAFE LIBSILVER_SAFE
#endif

#ifndef Weaver_USE_PTHREAD
	#define Weaver_USE_PTHREAD LIBSILVER_USE_PTHREAD
#endif

#ifndef Weaver_CACHE_ALIGN
	#define Weaver_CACHE_ALIGN _Alignas(cpu_hdi_size)
#endif

#ifndef Weaver_CALL_IMMEDIATE
	#define Weaver_CALL_IMMEDIATE true
#endif

#ifndef Weaver_RESUME_IMMEDIATE
	#define Weaver_RESUME_IMMEDIATE true
#endif

#ifndef Weaver_DEBUG
	#define Weaver_DEBUG LIBSILVER_DEBUG
#endif

#ifndef Weaver_SWAP_FIRST
	#define Weaver_SWAP_FIRST true
#endif
