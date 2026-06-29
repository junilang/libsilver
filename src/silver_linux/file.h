typedef int linux_fd;

constexpr linux_fd linux_stdin = 0;
constexpr linux_fd linux_stdout = 1;
constexpr linux_fd linux_stderr = 2;

LINUX_SYSCALL_GENERATE(linux_openat2, __NR_openat2,
	(linux_fd dirfd, Str path, struct open_how *how, uword size),
	dirfd, path, how, size
)

LINUX_SYSCALL_GENERATE(linux_close, __NR_close,
	(linux_fd fd), fd
)

LINUX_SYSCALL_GENERATE(linux_statx, __NR_statx,
	(linux_fd dirfd, Str path, uword flags, uword mask, struct statx *buffer),
	dirfd, path, flags, mask, buffer
)

LINUX_SYSCALL_GENERATE(linux_write, __NR_write,
	(linux_fd fd, ConstPtr buf, uword buf_size), fd, buf, buf_size
)

LINUX_SYSCALL_GENERATE(linux_read, __NR_read,
	(int fd, Ptr buf, unsigned long buf_size), fd, buf, buf_size
)
