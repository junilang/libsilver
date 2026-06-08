#if BUILD_NOLIBC

extern void (*__init_array_start[])() __attribute__((weak));
extern void (*__init_array_end[])() __attribute__((weak));

extern void (*__fini_array_start[])() __attribute__((weak));
extern void (*__fini_array_end[])() __attribute__((weak));

extern int main(int argc, Str *argv, Str *envp);

int silver_main(int argc, Str *argv, Str *envp) {
	{
		auto volatile it = __init_array_start;
		for (; it < __init_array_end; it++)
			(*it)();
	}

	int exit_code = main(argc, argv, envp);

	{
		auto volatile it = __fini_array_start;
		for (; it < __fini_array_end; it++)
			(*it)();
	}

	return 128;
}

[[gnu::used, gnu::visibility("default")]]
void __stack_chk_fail() {
	__builtin_trap();
}

#endif
