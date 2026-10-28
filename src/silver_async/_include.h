#if !LIBSILVER_ASYNC_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ASYNC_INCLUDE
	#define LIBSILVER_ASYNC_INCLUDE true

	#include "wait.h"
	#if __linux__
		#include "wait_linux.h"
	#else
		#error "unsupported platform"
	#endif

	#include "Async_decl.h"
	#include "AsyncRes.h"
	#include "AsyncRT.h"
	#include "AsyncTask.h"
	#include "AsyncFuture.h"

#endif
