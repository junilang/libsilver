LINUX_SYSCALL_GENERATE(linux_futex_wake, __NR_futex_wake,
	(Ptr addr, uword mask, uword nr, uword flags),
	addr, mask, nr, flags
)

LINUX_SYSCALL_GENERATE(linux_futex_wait, __NR_futex_wait,
	(
		Ptr addr, uword val, uword mask, uword flags,
		struct __kernel_timespec *timeout, uword clockid
	),
	addr, val, mask, flags, timeout, clockid
)
