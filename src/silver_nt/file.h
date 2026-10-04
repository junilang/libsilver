HANDLE nt_stdin = INVALID_HANDLE_VALUE;
HANDLE nt_stdout = INVALID_HANDLE_VALUE;
HANDLE nt_stderr = INVALID_HANDLE_VALUE;

[[gnu::constructor(110)]] void nt_get_std_handles__ctor() {
	/*
	WINBOOL res = SetConsoleCP(CP_UTF8);
	if (!res)
		os_panic();
	*/

	nt_stdin = GetStdHandle(STD_INPUT_HANDLE);
	if (nt_stdin == INVALID_HANDLE_VALUE)
		os_panic();

	nt_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (nt_stdout == INVALID_HANDLE_VALUE)
		os_panic();

	nt_stderr = GetStdHandle(STD_ERROR_HANDLE);
	if (nt_stderr == INVALID_HANDLE_VALUE)
		os_panic();
}
