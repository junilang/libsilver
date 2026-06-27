#define PANIC__LINE_(line) #line
#define PANIC__LINE(line) PANIC__LINE_(line)

#ifdef __linux__

	#define PANIC_HEADER \
		linux_write(linux_stderr, USTR("PANIC: "__FILE__":"PANIC__LINE(__LINE__)" in ")); \
		linux_write(linux_stderr, USTR(__func__)); \
		linux_write(linux_stderr, USTR(": "));

	#define PANIC(message) { \
		PANIC_HEADER \
		linux_write(linux_stderr, USTR(message"\n")); \
		__builtin_trap(); \
	}

	#define PANIC_PRINT(...) { \
		PANIC_HEADER \
		PRINT(Stderr, __VA_ARGS__,"\n"); \
		__builtin_trap(); \
	}

#else
	#error "unsupported system"

#endif

#if BUILD_DEBUG
	#define UNREACHABLE PANIC("unreachable")
#else
	#define UNREACHABLE { __builtin_unreachable(); }
#endif
