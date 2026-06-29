extern void os_panic_write(const ubyte *message, usize size);
[[noreturn]] extern void os_panic();

#define PANIC__LINE_(line) #line
#define PANIC__LINE(line) PANIC__LINE_(line)

#define PANIC_HEADER \
	os_panic_write(USTR("PANIC: "__FILE__":"PANIC__LINE(__LINE__)" in ")); \
	os_panic_write(USTR(__func__)); \
	os_panic_write(USTR(": "));

#define PANIC(message) { \
	PANIC_HEADER \
	os_panic_write(USTR(message"\n")); \
	os_panic(); \
}

#if BUILD_DEBUG
	#define UNREACHABLE PANIC("unreachable")
#else
	#define UNREACHABLE { __builtin_unreachable(); }
#endif
