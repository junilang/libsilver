typedef union {
	alignas(16) char align__;
	struct {
		int tid;
	};
} ZZThreadData;

[[noreturn]] void ZZthread(uword arg) {
	auto const this = (ZZThreadData*)arg;

	PRINTB(128, Stdout, "child thread: ",this->tid,"\n");

	PRINTB(128, Stdout, "setup tls\n");
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

	PRINTB(128, Stdout, "page size is ",page_size,"\n");

	const usize stack_size = page_size * 16;
	const usize guard_size = page_size;

	res = linux_mmap(nullptr, stack_size,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0
	);

	if (linux_mmap_iserror(res)) {
		PRINTB(128, Stdout, "mmap error: ",res,"\n");
		return 1;
	}

	auto stack_begin = (ubyte*)res;
	auto stack_end = stack_begin + stack_size;

	PRINTB(128, Stdout, "allocated stack[",stack_size,"] at ",stack_begin,"\n");

	auto thread_data = (ZZThreadData*)stack_begin;
	auto stack = (ubyte*)(thread_data + 1);

	/*
	res = linux_mprotect(stack, guard_size, PROT_NONE);
	if (res) {
		FPRINTB(128, Stdout, "mprotect error: ",res,"\n");
		return 1;
	}
	*/

	struct clone_args clone_args = {\
		.flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_CHILD_SETTID,
		.stack = (usize)stack,
		.stack_size = (usize)(stack_end - stack),
		.child_tid = (usize)&thread_data->tid
	};

	res = linux_clone3_safe(&clone_args, sizeof(clone_args), &ZZthread, (usize)thread_data);
	if (res < 0) {
		PRINTB(128, Stdout, "clone3 error: ",res,"\n");
	}

	PRINTB(128, Stdout, "parent\n");

	return 0;
}
