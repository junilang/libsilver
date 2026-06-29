#if !LIBSILVER_CORE_DEPENDS
	#define LIBSILVER_CORE_DEPENDS true

	#if BUILD_ASAN
		#include <sanitizer/asan_interface.h>
	#endif

#endif
