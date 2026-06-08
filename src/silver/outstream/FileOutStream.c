#ifdef __linux__

	void FileOutStream_write(int fd, ConstPtr buffer, usize buffer_size) {
		linux_write(fd, buffer, buffer_size);
	}

	void FileOutStream_flush(int fd) {}

	IOutStream_GENERATE_KNOWN(FileOutStream, int)

#else
	#error "unsupported platform"
#endif
