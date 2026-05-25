#if !LIBSILVER_ASYNC_DEPENDS
	#define LIBSILVER_ASYNC_DEPENDS true

	#undef __STRICT_ANSI__

	#include <unistd.h>
	#include <pthread.h>
	#include <sys/syscall.h>
	#include <linux/futex.h>
	#include <x86gprintrin.h>

#endif
