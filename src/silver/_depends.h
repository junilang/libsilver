#if !LIBSILVER_DEPENDS
	#define LIBSILVER_DEPENDS true

	#ifndef LIBSILVER_OS_INCLUDE
		#include "../silver_os.h"
	#endif

	#ifndef LIBSILVER_INIT_INCLUDE
		#include "../silver_init.h"
	#endif

	#if BUILD_ASAN
		#include <sanitizer/asan_interface.h>
	#endif

#endif
