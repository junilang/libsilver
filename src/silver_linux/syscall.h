#define LINUX_SYSCALL_TOO_MANY_ARGS _Pragma("GCC error \"LINUX_SYSCALL: too many arguments\"")

#define LINUX_SYSCALL_SETREG_X(N, arg) \
	register uword x##N __asm__(LINUX_SYSCALL_REG_##N) = (uword)(arg);

#define LINUX_SYSCALL_SETREG(...) __VA_OPT__(LINUX_SYSCALL_SETREG_0(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_0(arg, ...) \
	LINUX_SYSCALL_SETREG_X(0, arg) __VA_OPT__(LINUX_SYSCALL_SETREG_1(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_1(arg, ...) \
	LINUX_SYSCALL_SETREG_X(1, arg) __VA_OPT__(LINUX_SYSCALL_SETREG_2(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_2(arg, ...) \
	LINUX_SYSCALL_SETREG_X(2, arg) __VA_OPT__(LINUX_SYSCALL_SETREG_3(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_3(arg, ...) \
	LINUX_SYSCALL_SETREG_X(3, arg) __VA_OPT__(LINUX_SYSCALL_SETREG_4(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_4(arg, ...) \
	LINUX_SYSCALL_SETREG_X(4, arg) __VA_OPT__(LINUX_SYSCALL_SETREG_5(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_5(arg, ...) \
	LINUX_SYSCALL_SETREG_X(5, arg) __VA_OPT__(LINUX_SYSCALL_SETREG_6(__VA_ARGS__))

#define LINUX_SYSCALL_SETREG_6(arg, ...) \
	LINUX_SYSCALL_SETREG_X(6, arg) __VA_OPT__(LINUX_SYSCALL_TOO_MANY_ARGS)

#define LINUX_SYSCALL_INPUT_X(N) "r" (x##N)

#define LINUX_SYSCALL_INPUT(...) __VA_OPT__(LINUX_SYSCALL_INPUT_0(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_0(arg, ...) \
	LINUX_SYSCALL_INPUT_X(0) __VA_OPT__(, LINUX_SYSCALL_INPUT_1(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_1(arg, ...) \
	LINUX_SYSCALL_INPUT_X(1) __VA_OPT__(, LINUX_SYSCALL_INPUT_2(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_2(arg, ...) \
	LINUX_SYSCALL_INPUT_X(2) __VA_OPT__(, LINUX_SYSCALL_INPUT_3(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_3(arg, ...) \
	LINUX_SYSCALL_INPUT_X(3) __VA_OPT__(, LINUX_SYSCALL_INPUT_4(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_4(arg, ...) \
	LINUX_SYSCALL_INPUT_X(4) __VA_OPT__(, LINUX_SYSCALL_INPUT_5(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_5(arg, ...) \
	LINUX_SYSCALL_INPUT_X(5) __VA_OPT__(, LINUX_SYSCALL_INPUT_6(__VA_ARGS__))

#define LINUX_SYSCALL_INPUT_6(arg, ...) \
	LINUX_SYSCALL_INPUT_X(6) __VA_OPT__(, LINUX_SYSCALL_TOO_MANY_ARGS)

#define LINUX_SYSCALL_RESULT__(id) x##id
#define LINUX_SYSCALL_RESULT_(id) LINUX_SYSCALL_RESULT__(id)
#define LINUX_SYSCALL_RESULT LINUX_SYSCALL_RESULT_(LINUX_SYSCALL_REG_RESULT_NUM)

#define LINUX_SYSCALL_GENERATE(name, id, args, ...) \
	[[gnu::always_inline]] static inline \
	iword name args { \
		LINUX_SYSCALL_SETREG(id, __VA_ARGS__) \
		__asm__ volatile ( \
			"syscall" \
			: "=r" (LINUX_SYSCALL_RESULT) \
			: LINUX_SYSCALL_INPUT(id, __VA_ARGS__) \
			: LINUX_SYSCALL_CLOBBER_LIST \
		); \
		return (iword)LINUX_SYSCALL_RESULT; \
	}

#define LINUX_SYSCALL_GENERATE_NORETURN(name, id, args, ...) \
	[[gnu::always_inline, noreturn]] static inline \
	iword name args { \
		LINUX_SYSCALL_SETREG(id, __VA_ARGS__) \
		__asm__ volatile ( \
			"syscall" \
			: "=r" (LINUX_SYSCALL_RESULT) \
			: LINUX_SYSCALL_INPUT(id, __VA_ARGS__) \
			: LINUX_SYSCALL_CLOBBER_LIST \
		); \
		__builtin_trap(); \
	}
