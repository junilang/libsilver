#ifdef __linux__
	void FileOutStream_write(linux_fd fd, ConstPtr buffer, usize buffer_size) {
		linux_write(fd, buffer, buffer_size);
	}

	void FileOutStream_flush(linux_fd fd) {}

	IOutStream_GENERATE_KNOWN(FileOutStream, linux_fd)

	constexpr linux_fd Stdout = linux_stdout;
	constexpr linux_fd Stderr = linux_stderr;

#else
	#error "unsupported platform"
#endif
