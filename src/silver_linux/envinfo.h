typedef struct {
	#if LIBSILVER_SAFE
		bool initialized;
	#endif

	uword pagesz;
	uword vdso_addr;
	uword random_addr;
} linux_envinfo;

void linux_envinfo_fromauxv(linux_envinfo *this, ConstPtr auxv_addr) {
	const uword *auxv_entry = auxv_addr;
	for (; auxv_entry[0] != AT_NULL; auxv_entry += 2) {
		switch (auxv_entry[0]) {
			case AT_PAGESZ:
				this->pagesz = auxv_entry[1];
				break;
			case AT_SYSINFO_EHDR:
				this->vdso_addr = auxv_entry[1];
				break;
			case AT_RANDOM:
				this->random_addr = auxv_entry[1];
				break;
			default:
		}
	}

	#if LIBSILVER_SAFE
		this->initialized = true;
	#endif
}

linux_envinfo global_linux_envinfo__ = {};

[[gnu::always_inline]] static inline
uword env_pagesz() {
	#if LIBSILVER_SAFE
		ASSERT(
			global_linux_envinfo__.initialized
			, "envinfo not initialized"
		);
	#endif

	return global_linux_envinfo__.pagesz;
}
