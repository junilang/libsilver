constexpr int linux_stdin = 0;
constexpr int linux_stdout = 1;
constexpr int linux_stderr = 2;

typedef int linux_fd;

LINUX_SYSCALL_GENERATE(linux_openat2, __NR_openat2,
	(linux_fd dirfd, const char *path, struct open_how *how, uword size),
	dirfd, path, how, size
)

LINUX_SYSCALL_GENERATE(linux_close, __NR_close,
	(linux_fd fd), fd
)

LINUX_SYSCALL_GENERATE(linux_statx, __NR_statx,
	(linux_fd dirfd, const char *path, uword flags, uword mask, struct statx *buffer),
	dirfd, path, flags, mask, buffer
)

LINUX_SYSCALL_GENERATE(linux_write, __NR_write,
	(linux_fd fd, const void *buf, uword buf_size), fd, buf, buf_size
)

LINUX_SYSCALL_GENERATE(linux_read, __NR_read,
	(int fd, void *buf, unsigned long buf_size), fd, buf, buf_size
)
