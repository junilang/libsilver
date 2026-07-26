Str const *global_env__;
const struct { uword key; uword val; } *global_auxv__;

#if !BUILD_NOLIBC
	extern Str *environ;
	[[gnu::constructor(101)]] void setup_env_globals_ctor__() {
		Str *envp = environ;
		global_env__ = (Ptr)envp;
		Str *it = envp;
		while (*it) it++;
		it++;
		global_auxv__ = (Ptr)it;
		linux_envinfo_fromauxv(&global_linux_envinfo__, (Ptr)it);
	}

#else
	extern void (*__init_array_start[])() __attribute__((weak));
	extern void (*__init_array_end[])() __attribute__((weak));

	extern void (*__fini_array_start[])() __attribute__((weak));
	extern void (*__fini_array_end[])() __attribute__((weak));

	[[gnu::used, gnu::noinline, gnu::visibility("default")]]
	void __stack_chk_fail() {
		__builtin_trap();
	}

	[[gnu::used, gnu::visibility("default")]]
	int __libc_start_main(
		int (*main)(int, char**, char**),
		int argc, char **argv,
		void (*init_fn)(),
		void (*fini_fn)(),
		void (*rtld_fini_fn)(),
		void (*stack_end)
	) {
		char **envp = argv + (argc + 1);

		{
			global_env__ = (Ptr)envp;
			char **it = envp;
			while (*it) it++;
			it++;
			global_auxv__ = (Ptr)it;
			linux_envinfo_fromauxv(&global_linux_envinfo__, (Ptr)it);
		}


		if (init_fn) {
			init_fn();
		} else {
			auto it = __init_array_start;
			for (; it < __init_array_end; it++) {
				(*it)();
			}
		}

		int exit_code = main(argc, argv, envp);

		if (fini_fn) {
			fini_fn();
		} else {
			auto it = __fini_array_start;
			for (; it < __fini_array_end; it++) {
				(*it)();
			}
		}

		if (rtld_fini_fn) rtld_fini_fn();

		linux_exit(exit_code);
	}

#endif
