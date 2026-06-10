#define PANIC__LINE_(line) #line
#define PANIC__LINE(line) PANIC__LINE_(line)

#ifdef __linux__

	/*
	#define PANIC_HEADER \
		ubyte PANIC_header__[128]; \
		ubyte *PANIC_headerp__ = PANIC_header__; \
		PANIC_headerp__ =
		linux_write(linux_stderr, USTR("PANIC: "__FILE__":"PANIC__LINE(__LINE__)" in ")); \
		linux_write(linux_stderr, USTR(__func__)); \
		linux_write(linux_stderr, USTR(": "));

	#define PANIC(str) { \
		PANIC_HEADER \
		fputs(str, stderr); \
		fputc('\n', stderr); \
		fflush(stderr); \
		abort(); \
	}

	#define PANICF(...) { \
		PANIC_HEADER \
		Print(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
		fflush(stderr); \
		abort(); \
	}
	*/

	#define PANIC(...) { __builtin_trap(); }
	#define PANICF PANIC

#else
	#error "unsupported system"

#endif

#if BUILD_DEBUG
	#define UNREACHABLE PANIC("unreachable")
#else
	#define UNREACHABLE { __builtin_unreachable(); }
#endif
