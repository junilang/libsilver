#if !LIBSILVER_DEPENDS
	#define LIBSILVER_DEPENDS true

	#if BUILD_ASAN
		#include <sanitizer/asan_interface.h>
	#endif

#endif
