#if !LIBSILVER_LINUX_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_LINUX_INCLUDE
	#define LIBSILVER_LINUX_INCLUDE true

	#ifdef __x86_64__
		#include "x86_64.h"

	#else
		#error "unsupported cpu architecture"

	#endif

	#include "syscall.h"
	#include "process.h"
	#include "file.h"
	#include "futex.h"
	#include "memory.h"
#endif
