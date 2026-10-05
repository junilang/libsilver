#if !LIBSILVER_WEAVER_DEPENDS
	#define LIBSILVER_WEAVER_DEPENDS true

	#if !LIBSILVER_CORE_INCLUDE
		#include "../silver_core.h"
	#endif

	#if !LIBSILVER_ASYNC_INCLUDE
		#include "../silver_async.h"
	#endif

	#if WEAVER_USE_PTHREAD
		#include <pthread.h>
	#endif

#endif
