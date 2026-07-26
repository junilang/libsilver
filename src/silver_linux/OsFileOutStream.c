// abstraction over native os files using syscalls without buffering

OutStreamRes OsFileOutStream_write(linux_fd fd, ConstPtr buffer, usize buffer_size) {
	iword result = linux_write(fd, buffer, buffer_size);
	if (result < 0) return OutStreamRes_ErrInternal;

	return OutStreamRes_Ok;
}

OutStreamRes OsFileOutStream_flush(linux_fd fd) {
	return OutStreamRes_Ok;
}

OutStreamAttr OsFileOutStream_attr(linux_fd fd) {
	return 0;
}

IOutStream_GENERATE_KNOWN(OsFileOutStream, linux_fd)

#if OutStream_PTRTAG
	#define XCONST constexpr
#else
	#define XCONST const
#endif

XCONST OutStream Stdout = CONSTEXPR_OutStream_upcast(OsFileOutStream, linux_stdout);
XCONST OutStream Stderr = CONSTEXPR_OutStream_upcast(OsFileOutStream, linux_stderr);

#undef XCONSTPREFIX

OutStream os_panic_stream() {
	return Stderr;
}

OutStream os_stdout_stream() {
	return Stdout;
}

OutStream os_stderr_stream() {
	return Stderr;
}
