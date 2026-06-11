LINUX_SYSCALL_GENERATE(linux_mmap, __NR_mmap,
	(const void *addr, uword len, uword prot, uword flags, linux_fd fd, uword off),
	addr, len, prot, flags, fd, off
)

bool linux_mmap_iserror(iword result) {
	if (result < 0 && result > -4095)
		return true;

	return false;
}

LINUX_SYSCALL_GENERATE(linux_munmap, __NR_munmap,
	(void *addr, uword len), addr, len
)

LINUX_SYSCALL_GENERATE(linux_mprotect, __NR_mprotect,
	(void *addr, uword len, uword prot)
)

LINUX_SYSCALL_GENERATE(linux_mremap, __NR_mremap,
	(void *addr, uword old_len, uword new_len, uword flags, void *new_addr),
	addr, old_len, new_len, flags, new_addr
)
