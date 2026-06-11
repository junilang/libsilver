typedef struct {
	uword pagesz;
} linux_envinfo;

void linux_envinfo_fromauxv(linux_envinfo *this, void *auxv_addr) {
	uword *auxv_entry = auxv_addr;

	while (true) {
		switch (auxv_entry[0]) {
			case AT_NULL:
				goto break_loop;
			case AT_PAGESZ:
				this->pagesz = auxv_entry[1];
				break;
		}

		auxv_entry += 2;
	}
	break_loop:;
}

linux_envinfo global_linux_envinfo__ = {};

[[gnu::always_inline]] static inline
uword env_pagesz() {
	return global_linux_envinfo__.pagesz;
}

void load_envinfo(int argc, const char **argv) {
	const char **envp = argv + (argc + 1);
	while (*envp) {
		envp++;
	}
	envp++;
	linux_envinfo_fromauxv(&global_linux_envinfo__, envp);
}
