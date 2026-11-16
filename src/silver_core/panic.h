extern void os_panic_write(ConstPtr message, usize size);
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
#define PANIC_MSG_8(...) PANIC_MSG_ERROR_TOO_MANY_MESSAGES

#define PANIC_MSG_ERROR_TOO_MANY_MESSAGES _Pragma("GCC error \"PANIC: too many messages\"")

#define PANIC_MSG_(...) do { \
	const String msg[] = { __VA_OPT__(PANIC_MSG_0(__VA_ARGS__)) }; \
	panic_msg_(msg, _Countof(msg)); \
} while (0)

#define PANIC_PRINT_(...) do { \
	usize constexpr PANIC_PRINT_buf_size__ = (PANIC_BUFSIZE); \
	ubyte PANIC_PRINT_buf__[PANIC_PRINT_buf_size__]; \
	BufferSink PANIC_PRINT_buf_sink__; \
	BufferSink_init(&PANIC_PRINT_buf_sink__, PANIC_PRINT_buf__, PANIC_PRINT_buf_size__); \
	Sink const PANIC_PRINT_sink__ = BufferSink_asSink(&PANIC_PRINT_buf_sink__); \
	SinkRes const PANIC_PRINT_res__ = PRINT_RAW(PANIC_PRINT_sink__, __VA_ARGS__); \
	SinkRes_assert_(PANIC_PRINT_res__, PANIC_HEADER("PANIC_PRINT failed:")); \
	os_panic_write(PANIC_PRINT_buf__, PANIC_PRINT_buf_sink__.size); \
	os_panic(); \
} while (0)

#define ASSERT_MSG_(cond, ...) do { \
	if (!(cond)) PANIC_MSG_(__VA_ARGS__); \
} while (0)

#define ASSERT_PRINT_(cond, ...) do { \
	if (!(cond)) PANIC_PRINT_(__VA_ARGS__); \
} while (0)

#define PANIC_HEADER(msg) __func__, SOURCE_IDENTIFIER " " msg
#define PANIC_PRINT_HEADER(msg) __func__, " " SOURCE_IDENTIFIER " " msg

#define PANIC_MSG(...) PANIC_MSG_( \
	PANIC_HEADER("PANIC" __VA_OPT__(":")) __VA_OPT__(,__VA_ARGS__) \
)

#define PANIC_PRINT(...) PANIC_PRINT_( \
	PANIC_PRINT_HEADER("PANIC" __VA_OPT__(":")) __VA_OPT__(,__VA_ARGS__) \
)

#define ASSERT_MSG(cond, ...) ASSERT_MSG_( \
	cond, PANIC_HEADER("ASSERT(" #cond ")" __VA_OPT__(":")) __VA_OPT__(,__VA_ARGS__) \
)

#define ASSERT_PRINT(cond, ...) ASSERT_PRINT_( \
	cond, PANIC_PRINT_HEADER("ASSERT(" #cond ")" __VA_OPT__(":")) __VA_OPT__(,__VA_ARGS__) \
)

#define UNREACHABLE_GUARD(guard) do { \
	if (guard) {  \
		PANIC_MSG_(PANIC_HEADER("UNREACHABLE")); \
	} else { __builtin_unreachable(); } \
} while (0)

#ifndef UNREACHABLE_DEFAULT_GUARD
	#define UNREACHABLE_DEFAULT_GUARD LIBSILVER_SAFE
#endif

#define UNREACHABLE_ UNREACHABLE_GUARD(UNREACHABLE_DEFAULT_GUARD)

#define UNREACHABLE(...) UNREACHABLE_##__VA_OPT__(GUARD(__VA_ARGS__))

#define PANIC_ARGS ConstPtr const func_name, ConstPtr const header
#define PANIC_ARGS_FWD func_name, header

#define PANIC_RES_GENERATE(R, S) \
	[[noreturn, gnu::noinline]] \
	void R##_panic_( \
		R const res, PANIC_ARGS \
	) { \
		constexpr auto plen = __builtin_strlen(#R "_"); \
		constexpr auto bsize = 32; \
		ubyte buf[plen + bsize + 8] = {#R "_"}; \
		usize bend = plen; \
		if (res >= _Countof(S)) { \
			memcpy(buf + bend, STR("Aux(")); \
			bend += __builtin_strlen("Aux("); \
			goto put_num; \
		} \
		auto const s = S[res]; \
		if (s.size > bsize) { \
			memcpy(buf + bend, STR("\?\?(")); \
			bend += __builtin_strlen("\?\?("); \
			goto put_num; \
		} \
		memcpy(buf + bend, s.data, s.size); \
		bend += s.size; \
		if (0) put_num: { \
			auto const r = ullong_rawfmt(res, FLAG(IntFmt_ZeroOffset), buf + bend, bsize); \
			if (r < 0) \
				bend += IntFmtRes_put(r, buf + bend, bsize); \
			else \
				bend += (usize)r; \
			buf[bend++] = ')'; \
		} \
		const String msg[] = { \
			String_INIT(func_name), \
			String_INIT(header), \
			{ .data = buf, .size = bend } \
		}; \
		panic_msg_(msg, _Countof(msg)); \
	} \

#define ASSERT_RES_GENERATE(R) \
	void R##_assert_(R res, PANIC_ARGS) { \
		if (res) R##_panic_(res, PANIC_ARGS_FWD); \
	}

#define ASSERT_RES(R, res) \
	R##_assert_(res, PANIC_HEADER(#R "_ASSERT(" #res "):"))

#define PANIC PANIC_MSG
#define ASSERT ASSERT_MSG

#define PANICF PANIC_PRINT
#define ASSERTF ASSERT_PRINT
