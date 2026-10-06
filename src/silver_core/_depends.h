#if !LIBSILVER_CORE_DEPENDS
	#define LIBSILVER_CORE_DEPENDS true

	#if !LIBSILVER_CONFIG_INCLUDE
		#include "../silver_config.h"
	#endif

	#if BUILD_ASAN
		#include <sanitizer/asan_interface.h>
	#endif

#endif
