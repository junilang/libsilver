#if !LIBSILVER_ASYNC_DEPENDS
	#define LIBSILVER_ASYNC_DEPENDS true

	#include <sched.h>

	#ifdef __linux__
		#include <unistd.h>
		#include <pthread.h>
		#include <sys/syscall.h>
		#include <linux/futex.h>
		#include <signal.h>
		#include <liburing.h>

	#else
		#error "silver_async only supports Linux"

	#endif

	#ifdef __x86_64__
		#include <x86gprintrin.h>

	#endif

#endif
