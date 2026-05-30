#if !LIBSILVER_SH_DEPENDS
	#define LIBSILVER_SH_DEPENDS

	#ifdef __linux__
		#include <sys/wait.h>
	#else
		#error "unsupported platform"
	#endif

#endif
