#if !LIBSILVER_NT_DEPENDS
	#define LIBSILVER_NT_DEPENDS true

	#if !LIBSILVER_CORE_INCLUDE
		#include "../silver_core.h"
	#endif

	#define WIN32_LEAN_AND_MEAN true
	#define NOGDI true
	#define NOUSER true
	#define NOMINMAX true
		#include <windows.h>

	#undef WIN32_LEAN_AND_MEAN
	#undef NOGDI

#endif
