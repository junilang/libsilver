#if !LIBSILVER_DEPENDS
	#undef __STRICT_ANSI__

	#include <stdio.h>
	#include <stdint.h>
	#include <stddef.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbit.h>
	#include <stdatomic.h>
	#include <limits.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <sys/syscall.h>
	#include <linux/futex.h>
	#include <errno.h>

	#include <x86gprintrin.h>

	#define LIBSILVER_DEPENDS true

#endif
