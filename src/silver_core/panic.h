extern void os_panic_write(const ubyte *message, usize size);
[[noreturn]] extern void os_panic();

[[noreturn, gnu::noinline]]
static void PANIC_internal(ConstPtr msg1, ConstPtr msg2, ConstPtr msg3, ConstPtr msg4) {
	if (msg1) {
		os_panic_write(msg1, __builtin_strlen(msg1));
		os_panic_write(STR(" "));
	}
	if (msg2) {
		os_panic_write(msg2, __builtin_strlen(msg2));
		os_panic_write(STR(" "));
	}
	if (msg3) {
		os_panic_write(msg3, __builtin_strlen(msg3));
		os_panic_write(STR(" "));
	}
	if (msg4) {
		os_panic_write(msg4, __builtin_strlen(msg4));
		os_panic_write(STR(" "));
	}
	os_panic_write(STR("\n"));
	os_panic();
}

#define PANIC_LINE_(line) #line
#define PANIC_LINE(line) PANIC_LINE_(line)

#define PANIC_IDENTIFIER __FILE__":"PANIC_LINE(__LINE__)

#define PANIC_2_HAS(msg) msg
#define PANIC_1_HAS(msg, ...) msg, PANIC_2##__VA_OPT__(_HAS)(__VA_ARGS__)

#define PANIC_2(...) nullptr
#define PANIC_1(...) nullptr, nullptr

#define PANIC(...) {  \
	PANIC_internal( \
		__func__, PANIC_IDENTIFIER" PANIC:", \
		PANIC_1##__VA_OPT__(_HAS)(__VA_ARGS__) \
	); \
}

#if BUILD_DEBUG
	#define UNREACHABLE { \
		PANIC_internal(__func__, PANIC_IDENTIFIER" PANIC: UNREACHABLE", nullptr, nullptr); \
	}
#else
	#define UNREACHABLE { __builtin_unreachable(); }
#endif
