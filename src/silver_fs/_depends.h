#if !LIBSILVER_FS_DEPENDS
	#define LIBSILVER_FS_DEPENDS true

	#if !LIBSILVER_CORE_INCLUDE
		#include "../silver_core.h"
	#endif

	#if !LIBSILVER_OS_INCLUDE
		#include "../silver_os.h"
	#endif

#endif
