#if !LIBSILVER_DEPENDS
	#define LIBSILVER_DEPENDS true

	#ifdef __linux__
		#if !LIBSILVER_LINUX_INCLUDE
			#error "silver_linux not included"
		#endif

	#else
		#error "unsupported platform"

	#endif

#endif
