void os_panic_write(const ubyte *message, usize size) {
	linux_write(linux_stderr, message, size);
}

[[noreturn]] void os_panic() {
	__builtin_trap();
}
