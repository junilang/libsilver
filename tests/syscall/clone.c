[[noreturn]] int ZZthread() {
	FPRINTB(128, Stdout, "child thread\n");
	linux_exit(0);
}

int ZZentry(SilverTestContext *ctx) {
	iword res;

	/*
	struct sysinfo sysinfo;
	res = linux_sysinfo(&sysinfo);
	if (res < 0) {
		FPRINTB(128, Stdout, "sysinfo error: ",res,"\n");
		return 1;
	}

	const usize page_size = sysinfo.mem_unit;
	*/

	constexpr usize page_size = 4096;

	FPRINTB(128, Stdout, "page size is ",page_size,"\n");

	const usize stack_size = page_size * 16;
	const usize guard_size = page_size;

	res = linux_mmap(nullptr, stack_size,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0
	);

	if (linux_mmap_iserror(res)) {
		FPRINTB(128, Stdout, "mmap error: ",res,"\n");
		return 1;
	}

	auto stack = (Ptr)res;
	FPRINTB(128, Stdout, "allocated stack[",stack_size,"] at ",(usize)stack,"\n");

	/*
	res = linux_mprotect(stack, guard_size, PROT_NONE);
	if (res) {
		FPRINTB(128, Stdout, "mprotect error: ",res,"\n");
		return 1;
	}
	*/

	struct clone_args clone_args = {\
		.flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD,
		.stack = (usize)stack,
		.stack_size = stack_size,
	};

	res = linux_clone3(&clone_args, sizeof(clone_args));
	if (res == 0) {
		ZZthread();
	}

	if (res < 0) {
		FPRINTB(128, Stdout, "clone3 error: ",res,"\n");
	}

	FPRINTB(128, Stdout, "parent\n");

	return 0;
}
