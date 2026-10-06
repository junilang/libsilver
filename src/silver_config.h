#ifndef LIBSILVER_CONFIG_INCLUDE
	#define LIBSILVER_CONFIG_INCLUDE true

	#ifndef LIBSILVER_DEBUG
		#define LIBSILVER_DEBUG BUILD_DEBUG
	#endif

	#ifndef LIBSILVER_SAFE
		// enable safe mode by default
		#ifndef BUILD_SAFE
			#define LIBSILVER_SAFE true
		#else
			#define LIBSILVER_SAFE BUILD_SAFE
		#endif
	#endif

	#ifndef LIBSILVER_NOLIBC
		#define LIBSILVER_NOLIBC false
	#endif

	#ifndef LIBSILVER_USE_ASAN
		#define LIBSILVER_USE_ASAN BUILD_ASAN
	#endif

	#ifndef LIBSILVER_USE_TSAN
		#define LIBSILVER_USE_TSAN BUILD_TSAN
	#endif

	#ifndef LIBSILVER_USE_UBSAN
		#define LIBSILVER_USE_UBSAN BUILD_UBSAN
	#endif

	#ifndef LIBSILVER_USE_TLS
		#define LIBSILVER_USE_TLS (!LIBSILVER_NOLIBC)
	#elif LIBSILVER_USE_TLS && LIBSILVER_NOLIBC
		#warning "TLS not supported without libc"
	#endif

	#ifndef LIBSILVER_USE_PTHREAD
		#define LIBSILVER_USE_PTHREAD (!LIBSILVER_NOLIBC)
	#elif LIBSILVER_USE_PTHREAD && LIBSILVER_NOLIBC
		#warning "pthread without libc"
	#endif

#endif
