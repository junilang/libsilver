#if !LIBSILVER_ASYNC_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ASYNC_INCLUDE
	#define LIBSILVER_ASYNC_INCLUDE true

	#include "Spinlock.h"
	#include "Async.h"
	#include "AsyncFuture.h"
	#include "AsyncTask.h"
	#include "AsyncRT.h"
	#include "async_helper.h"

	#include "weaver/Weaver.c"

#endif
