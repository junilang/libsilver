#if !LIBSILVER_ASYNC_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ASYNC_INCLUDE
	#define LIBSILVER_ASYNC_INCLUDE true

	#if 0

	#include "cpu.h"

	#ifdef __linux__
		#include "umtx_linux.h"

	#else
		#error "unsupported platform"

	#endif

	#include "Async.h"

	#include "AsyncRT.h"
	#include "AsyncTask.h"
	#include "AsyncFuture.h"

	#include "AsyncFuture_Mtx.c"
	#include "AsyncFuture_Task.c"
	#include "AsyncFuture.c"

	//#include "async_helper.h"

	#include "weaver/Weaver.c"

	#include "async_helper/_include.h"

	#endif

#endif
