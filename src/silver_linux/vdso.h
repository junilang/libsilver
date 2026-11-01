#if LIBSILVER_LINUX_USE_VDSO
	iword (*linux_clock_gettime_fast)(uword clockid, linux_timespec *out_time);
	iword (*linux_clock_getres_fast)(uword clockid, linux_timespec *out_res);

	// mostly stolen from parse_vdso.c

	u64 linux_elf_hash(const ubyte *data, usize size) {
		u64 h = 0;
		u64 g;

		auto const end = data + size;
		for (; data < end; data++) {
			h = (h << 4) + (*data);
			g = h & 0xf0000000;
			if (g) h ^= g >> 24;
			h &= ~g;
		}

		return h;
	}

	u32 linux_gnu_hash(const ubyte *data, usize size) {
		u32 h = 5381;

		auto const end = data + size;
		for (; data < end; data++)
			h += h * 32 + (*data);

		return h;
	}

	typedef struct {
		Elf64_Word *buckets;
		Elf64_Word *chains;
		Elf64_Word buckets_size;
		Elf64_Word chains_size;
	} linux_vdso_info;

	void linux_vdso_sym(const linux_vdso_info *info, const ubyte *name, usize name_size) {

	}

	void linux_setup_vdso(uword vdso_base) {
		// parse elf header
		auto const ehdr = (const Elf64_Ehdr*)vdso_base;

		const Elf64_Dyn *dyn = nullptr;
		uword dyn_base = 0;

		auto phdr = (const Elf64_Phdr*)(vdso_base + ehdr->e_phoff);
		auto const phdr_end = phdr + ehdr->e_phnum;

		for (; phdr < phdr_end; phdr++) {
			switch (phdr->p_type) {
				case PT_LOAD:
					if (!dyn_base)
						dyn_base =
							(vdso_base + phdr->p_offset) - (usize)phdr->p_vaddr
						;
					break;

				case PT_DYNAMIC:
					dyn = (ConstPtr)(vdso_base + phdr->p_offset);
					break;

				default:
			}
		}

		if (!dyn_base || !dyn)
			goto error;

		ConstPtr strtab = nullptr;
		const Elf64_Sym *sym = nullptr;
		const Elf64_Word *hash = nullptr;

		for (; dyn->d_tag != DT_NULL; dyn++) {
			switch (dyn->d_tag) {
				case DT_STRTAB:
					strtab = (ConstPtr)(dyn_base + (usize)dyn->d_un.d_ptr);
					break;
				case DT_SYMTAB:
					sym = (ConstPtr)(dyn_base + (usize)dyn->d_un.d_ptr);
					break;
				case DT_HASH:
					hash = (ConstPtr)(dyn_base + (usize)dyn->d_un.d_ptr);
					break;
				default:;
			}
		}

		if (!strtab || !sym || !hash)
			goto error;


		return;

		error:;
		os_panic_write(STR("failed to parse vdso"));
		os_panic();
	}

#else
	[[deprecated("not using vdso")]]
	iword linux_clock_gettime_fast(uword clockid, linux_timespec *out_time) {
		return linux_clock_gettime(clockid, out_time);
	}

	[[deprecated("not using vdso")]]
	iword linux_clock_getres_fast(uword clockid, linux_timespec *out_res) {
		return linux_clock_gettime(clockid, out_res);
	}
#endif
