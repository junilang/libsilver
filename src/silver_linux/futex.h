LINUX_SYSCALL_GENERATE(linux_futex_wake, __NR_futex_wake,
	(ConstPtr addr, uword mask, int nr, uword flags),
	addr, mask, nr, flags
)

LINUX_SYSCALL_GENERATE(linux_futex_wait, __NR_futex_wait,
	(
		ConstPtr addr, uword val, uword mask, uword flags,
		const linux_timespec *timeout, uword clockid
	),
	addr, val, mask, flags, timeout, clockid
)
