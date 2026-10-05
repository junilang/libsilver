typedef struct __kernel_timespec linux_timespec;

LINUX_SYSCALL_GENERATE(linux_clock_gettime, __NR_clock_gettime,
	(uword clockid, linux_timespec *out_time),
	clockid, out_time
)

LINUX_SYSCALL_GENERATE(linux_clock_getres, __NR_clock_getres,
	(uword clockid, linux_timespec *out_res),
	clockid, out_res
)

LINUX_SYSCALL_GENERATE(linux_clock_settime, __NR_clock_gettime,
	(uword clockid, const linux_timespec *out_time),
	clockid, out_time
)

LINUX_SYSCALL_GENERATE(linux_clock_nanosleep, __NR_clock_gettime,
	(
		uword clockid, uword flags,
		const linux_timespec *timeout,
		linux_timespec *remain
	),
	clockid, flags, timeout, remain
)
