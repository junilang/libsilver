#if !LIBSILVER_ASYNC_DEPENDS
	#define LIBSILVER_ASYNC_DEPENDS true

	#if !LIBSILVER_CORE_INCLUDE
		#include "../silver_core.h"
	#endif

	#if !LIBSILVER_OS_INCLUDE
		#include "../silver_os.h"
	#endif

#endif
