#if !LIBSILVER_ASYNC_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ASYNC_INCLUDE
	#define LIBSILVER_ASYNC_INCLUDE true

	#include "atomic.h"
	#include "atomic_types.h"
	#include "Arc.h"

	#include "Async.h"
	#include "AsyncRT.h"
	#include "AsyncTask.h"

#endif
