extern void os_panic_write(const ubyte *message, usize size);
[[noreturn]] extern void os_panic();

#ifndef PANIC_BUFSIZE
	#define PANIC_BUFSIZE 256
#endif

[[noreturn, gnu::noinline]]
void panic_msg_(const String *const msg, usize const msg_size) {
	if (!msg || !msg_size || msg_size > 8) {
		os_panic_write(STR("PANIC_internal: invalid input"));
		goto panic;
	}

	constexpr usize buf_size = (PANIC_BUFSIZE);
	constexpr usize real_size = buf_size + 8; // extra bytes for newline and space
	ubyte buf[real_size];
	usize buf_index = 0;

	for (usize i = 0; i < msg_size; i++) {
		auto const mdata = msg[i].data;
		auto msize = msg[i].size;

		if (!mdata || !msize)
			continue;

		if (i) buf[buf_index++] = ' ';

		usize buf_target;
		if (chkdadd(buf_index, msize, &buf_target))
			goto overflow;

		if (buf_target >= buf_size) {
			auto const size = (usize)(buf_size - buf_index);
			memcpy(buf + buf_index, mdata, size);
			buf_index = buf_size;
			break;
		}

		memcpy(buf + buf_index, mdata, msize);
		buf_index = buf_target;

		if (buf_size - buf_index < 2)
			break;
	}

	buf[buf_index++] = '\n';

	if (buf_index > real_size) overflow: {
		os_panic_write(STR("panig_msg_: overflow"));
		goto panic;
	}

	os_panic_write(buf, buf_index);

	panic:;
	os_panic();
}


#define SOURCE_LINE__(line) #line
#define SOURCE_LINE_(line) SOURCE_LINE__(line)
#define SOURCE_LINE SOURCE_LINE_(__LINE__)

#define SOURCE_IDENTIFIER __FILE__ ":" SOURCE_LINE

#define PANIC_MSG_0(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_1(__VA_ARGS__))
#define PANIC_MSG_1(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_2(__VA_ARGS__))
#define PANIC_MSG_2(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_3(__VA_ARGS__))
#define PANIC_MSG_3(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_4(__VA_ARGS__))
#define PANIC_MSG_4(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_5(__VA_ARGS__))
#define PANIC_MSG_5(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_6(__VA_ARGS__))
#define PANIC_MSG_6(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_7(__VA_ARGS__))
#define PANIC_MSG_7(M, ...) String_INIT(M), __VA_OPT__(PANIC_MSG_8(__VA_ARGS__))
#define PANIC_MSG_8(M, ...) PANIC_MSG_ERROR_TOO_MANY_MESSAGES

#define PANIC_MSG_(...) { \
	const String msg[] = { __VA_OPT__(PANIC_MSG_0(__VA_ARGS__)) }; \
	panic_msg_(msg, COUNTOF(msg)); \
}

#define PANIC_PRINT_(...) { \
	OutStreamRes PANIC_PRINT_res__; \
	PRINT_BUFFERED(&PANIC_PRINT_res__, (PANIC_BUFSIZE), os_panic_stream(), __VA_ARGS__); \
	OutStreamRes_assert_(PANIC_PRINT_res__, __func__, SOURCE_IDENTIFIER " PANIC_PRINT failed:"); \
	os_panic(); \
}

#define ASSERT_MSG_(cond, ...) { \
	if (!(cond)) { PANIC_MSG_(__VA_ARGS__) } \
}

#define ASSERT_PRINT_(cond, ...) { \
	if (!(cond)) { PANIC_PRINT_(__VA_ARGS__) } \
}

#define PANIC_MSG(...) PANIC_MSG_( \
	__func__, SOURCE_IDENTIFIER " PANIC" __VA_OPT__(":", __VA_ARGS__) \
)

#define PANIC_PRINT(...) PANIC_PRINT_( \
	__func__, " " SOURCE_IDENTIFIER " PANIC" __VA_OPT__(": ", __VA_ARGS__) \
) \

#define ASSERT_MSG(cond, ...) ASSERT_MSG_( \
	cond, __func__, SOURCE_IDENTIFIER " ASSERT(" #cond ")" __VA_OPT__(":", __VA_ARGS__) \
)

#define ASSERT_PRINT(cond, ...) ASSERT_PRINT_( \
	cond, __func__, " " SOURCE_IDENTIFIER " ASSERT(" #cond ")" __VA_OPT__(": ", __VA_ARGS__) \
)

#define UNREACHABLE_GUARD(guard) { \
	if (guard) {  \
		PANIC_MSG_(__func__, SOURCE_IDENTIFIER " UNREACHABLE"); \
	} else { __builtin_unreachable(); } \
};

#ifndef UNREACHABLE_DEFAULT_GUARD
	#define UNREACHABLE_DEFAULT_GUARD LIBSILVER_SAFE
#endif

#define UNREACHABLE_ UNREACHABLE_GUARD(UNREACHABLE_DEFAULT_GUARD)

#define UNREACHABLE(...) UNREACHABLE_##__VA_OPT__(GUARD(__VA_ARGS__))

#define PANIC PANIC_MSG
#define ASSERT ASSERT_MSG

#define PANICF PANIC_PRINT
#define ASSERTF ASSERT_PRINT
