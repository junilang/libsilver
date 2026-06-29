LINUX_SYSCALL_GENERATE(linux_mmap, __NR_mmap,
	(ConstPtr addr, uword len, uword prot, uword flags, linux_fd fd, uword off),
	addr, len, prot, flags, fd, off
)

bool linux_mmap_iserror(iword result) {
	if (result < 0 && result > -4095)
		return true;

	return false;
}

LINUX_SYSCALL_GENERATE(linux_munmap, __NR_munmap,
	(Ptr addr, uword len), addr, len
)

LINUX_SYSCALL_GENERATE(linux_mprotect, __NR_mprotect,
	(Ptr addr, uword len, uword prot)
)

LINUX_SYSCALL_GENERATE(linux_mremap, __NR_mremap,
	(Ptr addr, uword old_len, uword new_len, uword flags, void *new_addr),
	addr, old_len, new_len, flags, new_addr
)
