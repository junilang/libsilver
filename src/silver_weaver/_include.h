#if !LIBSILVER_WEAVER_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_WEAVER_INCLUDE
	#define LIBSILVER_WEAVER_INCLUDE true

	#include "config.h"
	#include "WeaverQueue.h"
	#include "WeaverThread.h"
	#include "Weaver.h"
	#include "WeaverThread.c"

#endif
