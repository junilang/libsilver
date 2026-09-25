#if !LIBSILVER_NT_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_NT_INCLUDE
	#define LIBSILVER_NT_INCLUDE true

	#if !LIBSILVER_OS_INCLUDE
		#define LIBSILVER_OS_INCLUDE true
	#else
		#error "only one system interface can be included"
	#endif

	#ifdef __x86_64__
		#include "x86_64.h"
	#else
		#error "unsupported cpu architecture"
	#endif

	#include "runtime.h"
	#include "panic.h"
	#include "file.h"

	#include "OsFileOutStream.c"

#endif
