OutStreamRes OsFileOutStream_write(i32 fd, ConstPtr buffer, usize buffer_size) {
	auto res = WriteFile(
		(HANDLE)(iword)fd, buffer, (DWORD)buffer_size, nullptr, nullptr
	);

	if (!res) return OutStreamRes_ErrUnknown;
	return OutStreamRes_Ok;
}

OutStreamRes OsFileOutStream_flush(i32 fd) {
	return OutStreamRes_Ok;
}

OutStreamAttr OsFileOutStream_attr(i32 fd) {
	return 0;
}

IOutStream_GENERATE_KNOWN(OsFileOutStream, i32)

OutStream os_stdout_stream() {
	HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hnd == INVALID_HANDLE_VALUE)
		os_panic();
	return OsFileOutStream_upcast((i32)(iword)hnd);
}

OutStream os_stderr_stream() {
	HANDLE hnd = GetStdHandle(STD_ERROR_HANDLE);
	if (hnd == INVALID_HANDLE_VALUE)
		os_panic();
	return OsFileOutStream_upcast((i32)(iword)hnd);
}

OutStream os_panic_stream() {
	return os_stderr_stream();
}

OutStream Stdout = NullOutStream;
OutStream Stderr = NullOutStream;

[[gnu::constructor(110)]] void nt_setup_std_streams__ctor() {
	Stdout = os_stdout_stream();
	Stderr = os_stderr_stream();
}
