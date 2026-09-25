void os_panic_write(const ubyte *message, usize size) {
	auto file = GetStdHandle(STD_ERROR_HANDLE);
	if (file == INVALID_HANDLE_VALUE)
		__builtin_trap();
	auto res = WriteFile(file, message, (DWORD)size, nullptr, nullptr);
	if (!res)
		__builtin_trap();
}

[[noreturn]] void os_panic() {
	__builtin_trap();
}
