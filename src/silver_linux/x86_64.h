#define LINUX_SYSCALL_REG_0 "rax"
#define LINUX_SYSCALL_REG_1 "rdi"
#define LINUX_SYSCALL_REG_2 "rsi"
#define LINUX_SYSCALL_REG_3 "rdx"
#define LINUX_SYSCALL_REG_4 "r10"
#define LINUX_SYSCALL_REG_5 "r8"
#define LINUX_SYSCALL_REG_6 "r9"

#define LINUX_SYSCALL_REG_RESULT "rax"
#define LINUX_SYSCALL_REG_RESULT_NUM 0

#define LINUX_SYSCALL_CLOBBER_LIST "rcx", "r11", "memory"

// safe wrapper around the clone3 syscall that
// handles the child and parent behavior in assembly
// by invoking the child_entry function

[[gnu::always_inline]] static inline
iword linux_clone3_safe(struct clone_args *args, uword size, void (*child_entry)(uword), uword child_arg) {
	register uword x0 __asm__("rax") = (uword)__NR_clone3;
	register uword x1 __asm__("rdi") = (uword)args;
	register uword x2 __asm__("rsi") = (uword)size;
	register uword x3 __asm__("rdx") = (uword)child_entry;
	register uword x4 __asm__("r10") = (uword)child_arg;

	__asm__ volatile (
		"syscall\n"
		"test %%rax, %%rax \n" // test syscall result
		"jnz 1f \n" // if not zero, return
		"mov %%r10, %%rdi \n" // set rdi to entry_arg (first arg to entry)
		"call *%%rdx \n" // call entry
		"ud2 \n" // trigger sigill if entry returns
		"1: \n"
		: "=r" (x0)
		: "r" (x0), "r" (x1), "r" (x2), "r" (x3), "r" (x4)
		: LINUX_SYSCALL_CLOBBER_LIST
	);

	return (iword)x0;
}
