#if !LIBSILVER_LINUX_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_LINUX_INCLUDE
	#define LIBSILVER_LINUX_INCLUDE true

	#ifdef __x86_64__
		#include "x86_64.h"

	#else
		#error "unsupported platform"

	#endif

	#include "syscall.h"
	#include "fcntl.h"
#endif
