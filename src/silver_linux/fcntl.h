LINUX_SYSCALL_GENERATE(linux_openat2, __NR_openat2,
	(int dirfd, const char *path, struct open_how *how, __kernel_size_t size),
	dirfd, path, how, size
)

LINUX_SYSCALL_GENERATE(linux_close, __NR_close,
	(int fd), fd
)

LINUX_SYSCALL_GENERATE(linux_statx, __NR_statx,
	(int dirfd, const char *path, int flags, unsigned int mask, struct statx *buffer),
	dirfd, path, flags, mask, buffer
)

LINUX_SYSCALL_GENERATE(linux_write, __NR_write,
	(int fd, const void *buf, __kernel_size_t buf_size), fd, buf, buf_size
)

LINUX_SYSCALL_GENERATE(linux_read, __NR_read,
	(int fd, void *buf, __kernel_size_t buf_size), fd, buf, buf_size
)

constexpr int linux_stdin = 0;
constexpr int linux_stdout = 1;
constexpr int linux_stderr = 2;
