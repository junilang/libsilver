int ZZentry(SilverTestContext *ctx) {
	struct open_how open_args = {
		.flags = O_RDONLY
	};

	linux_fd fd;

	auto res = linux_openat2(AT_FDCWD, "tests/syscall/open.txt", &open_args, sizeof(open_args));
	if (res < 0) {
		PRINTB(128, Stdout, "openat2 error: ",res,"\n");
		return 1;
	}

	fd = (linux_fd)res;

	ubyte data[128];
	res = linux_read(fd, data, sizeof(data));
	if (res < 0) {
		PRINTB(128, Stdout, "read error: ",res,"\n");
		return 1;
	}

	usize data_size = (usize)res;

	PRINTB(128, Stdout, "read ",data_size," bytes: \n");
	PRINT(Stdout, LITERAL(String, .data=data,.size=data_size), "\n");

	res = linux_close(fd);
	if (res < 0) {
		PRINTB(128, Stdout, "close error: ",res,"\n");
		return 1;
	}

	return SilverTest_OK;
}
