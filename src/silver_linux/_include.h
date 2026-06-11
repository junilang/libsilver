#if !LIBSILVER_LINUX_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_LINUX_INCLUDE
	#define LIBSILVER_LINUX_INCLUDE true

	#ifndef LIBSILVER_OS_INCLUDE
		#define LIBSILVER_OS_INCLUDE true
	#else
		#error "only one system interface can be included"
	#endif

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
	#include "envinfo.h"
#endif
