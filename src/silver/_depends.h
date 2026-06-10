#if !LIBSILVER_DEPENDS
	#define LIBSILVER_DEPENDS true

	#ifndef LIBSILVER_OS_INCLUDE
		#include "../silver_os.h"
	#endif

	#if BUILD_ASAN
		#include <sanitizer/asan_interface.h>
	#endif

#endif
