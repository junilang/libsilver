#ifndef Weaver_SAFE
	#define Weaver_SAFE LIBSILVER_SAFE
#endif

#ifndef Weaver_DEBUG
	#define Weaver_DEBUG LIBSILVER_DEBUG
#endif

#ifndef Weaver_USE_PTHREAD
	#define Weaver_USE_PTHREAD LIBSILVER_USE_PTHREAD
#endif

#ifndef Weaver_CACHE_ALIGNMENT
	#define Weaver_CACHE_ALIGNMENT CPU_HDI_SIZE
#endif

#if Weaver_CACHE_ALIGNMENT
	#define Weaver_CACHE_ALIGN _Alignas(Weaver_CACHE_ALIGNMENT)
#else
	#define Weaver_CACHE_ALIGN
#endif

#ifndef Weaver_CALL_IMMEDIATE
	#define Weaver_CALL_IMMEDIATE true
#endif

#ifndef Weaver_RESUME_IMMEDIATE
	#define Weaver_RESUME_IMMEDIATE true
#endif

#ifndef Weaver_SWAP_FIRST
	#define Weaver_SWAP_FIRST true
#endif

#ifndef Weaver_FAST_FETCH
	#define Weaver_FAST_FETCH true
#endif
