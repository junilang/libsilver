#if !LIBSILVER_ASYNC_DEPENDS
	#define LIBSILVER_ASYNC_DEPENDS true

	#ifdef __linux__
		// need to set __USE_MISC to include "syscall"
		#ifdef __STRICT_ANSI__
			#define __USE_MISC
		#endif

		#include <unistd.h>
		#include <pthread.h>
		#include <sys/syscall.h>
		#include <linux/futex.h>
		#include <x86gprintrin.h>

	#else
		#error "silver_async only supports Linux"

	#endif

#endif
