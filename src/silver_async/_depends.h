#if !LIBSILVER_ASYNC_DEPENDS
	#define LIBSILVER_ASYNC_DEPENDS true

	#include <sched.h>

	#ifdef __linux__
		#include <unistd.h>
		#include <errno.h>
		#include <pthread.h>
		#include <sys/syscall.h>
		#include <linux/futex.h>
		#include <signal.h>
		#include <liburing.h>

	#else
		#error "unsupported platform"

	#endif

	#ifdef __x86_64__
		#include <x86gprintrin.h>

	#endif

#endif
