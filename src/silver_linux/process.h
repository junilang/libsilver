LINUX_SYSCALL_GENERATE_NORETURN(linux_exit, __NR_exit,
	(int status), status
)

LINUX_SYSCALL_GENERATE(linux_clone3, __NR_clone3,
	(struct clone_args *args, uword size), args, size
)

LINUX_SYSCALL_GENERATE(linux_waitid, __NR_waitid,
	(uword which, uword pid, struct siginfo *out_info, uword options, void *rusage),
	which, pid, out_info, options, rusage
)

LINUX_SYSCALL_GENERATE(linux_sched_yield, __NR_sched_yield, ())

LINUX_SYSCALL_GENERATE(linux_sysinfo, __NR_sysinfo, (struct sysinfo *out_info), out_info)
