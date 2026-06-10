#if !LIBSILVER_LINUX_DEPENDS
	#define LIBSILVER_LINUX_DEPENDS true

	#include <linux/unistd.h>
	#include <linux/errno.h>
	#include <linux/fcntl.h>
	#include <linux/stat.h>
	#include <linux/openat2.h>
	#include <linux/futex.h>
	#include <linux/sched.h>
	#include <linux/mman.h>
	#include <linux/signal.h>
	#include <linux/sysinfo.h>
	#include <linux/time_types.h>

#endif
