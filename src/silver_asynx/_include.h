#if !LIBSILVER_ASYNX_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_ASYNX_INCLUDE
	#define LIBSVILER_ASYNX_INCLUDE true

	#include "config.h"

	#if !Asynx_NO_SUGAR
		#include "sugar.h"
	#endif

	#include "layout.h"
	#include "spawn.h"
	#include "create.h"
	#include "flow.h"
	#include "call.h"
	#include "fork.h"
	#include "error.h"
	#include "base.h"
#endif
