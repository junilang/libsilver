#if !LIBSILVER_ASYNC_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ASYNC_INCLUDE
	#define LIBSILVER_ASYNC_INCLUDE true

	#include "cpu.h"
	#include "atomic.h"
	#include "atomic_types.h"
	#include "Arc.h"

	#include "wait.h"
	#if __linux__
		#include "wait_linux.h"
	#else
		#error "unsupported platform"
	#endif

	#include "Async.h"
	#include "AsyncRT.h"
	#include "AsyncTask.h"
	#include "AsyncFuture.h"
	#include "AsyncFuture_Mutex.h"
	#include "AsyncFuture_Task.h"
	#include "AsyncFuture.c"

#endif
