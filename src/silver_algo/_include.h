#if !LIBSILVER_ALGO_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ALGO_INCLUDE
	#define LIBSILVER_ALGO_INCLUDE true

	#include "crc32.h"
	#include "fnv1a.h"
	#include "splitmix64.h"
#endif
