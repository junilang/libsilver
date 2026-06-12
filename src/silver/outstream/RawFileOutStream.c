// abstraction over native os files using syscalls without buffering

#ifdef __linux__
	void RawFileOutStream_write(linux_fd fd, ConstPtr buffer, usize buffer_size) {
		linux_write(fd, buffer, buffer_size);
	}

	void RawFileOutStream_flush(linux_fd fd) {}

	OutStreamAttr RawFileOutStream_attr(linux_fd fd) {
		return 0;
	}

	IOutStream_GENERATE_KNOWN(RawFileOutStream, linux_fd)

	#if OutStream_PTRTAG
		constexpr OutStream Stdout = CONSTEXPR_OutStream_upcast(RawFileOutStream, linux_stdout);
		constexpr OutStream Stderr = CONSTEXPR_OutStream_upcast(RawFileOutStream, linux_stderr);

	#else
		const OutStream Stdout = CONSTEXPR_OutStream_upcast(RawFileOutStream, linux_stdout);
		const OutStream Stderr = CONSTEXPR_OutStream_upcast(RawFileOutStream, linux_stderr);

	#endif

#else
	#error "unsupported platform"
#endif
