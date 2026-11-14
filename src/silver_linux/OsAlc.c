typedef struct {
	Ptr map_base;
	usize map_size;
	ubyte data[];
} OsAlc_Header;

auto constexpr OsAlc_minalign = _Alignof(usize);
auto constexpr OsAlc_dataoffset = __builtin_offsetof(OsAlc_Header, data);

AlcPtr OsAlc_new(Ptr this, AlcReq req, Ptr aux) {
	auto const page_size = env_pagesz();

	ConstPtr const hint = nullptr;

	ualign align = AlcReq_getalign(req);
	if (align > page_size)
		return AlcPtr_set(AlcRes_ErrInvalidAlign);

	usize const req_size = usize_align(FIELD_GET(AlcSize, req), align);

	auto const base_offset = usize_align(OsAlc_dataoffset, align);
	auto const map_size = usize_align(base_offset + req_size, page_size);
	auto const res = linux_mmap(hint, map_size,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0
	);

	if (linux_mmap_iserror(res)) {
		switch (-res) {
			case ENOMEM:
				return AlcPtr_set(AlcRes_ErrNoMemory);
			default:
				return AlcPtr_set(AlcRes_ErrUnknown);
		}
	}

	auto const map_base = (usize)res;
	auto const data = map_base + base_offset;

	auto const header = (OsAlc_Header*)(data - OsAlc_dataoffset);
	header->map_base = (Ptr)map_base;
	header->map_size = map_size;

	if (aux)
		*(usize*)aux = map_size;

	return (AlcPtr)data;
}

AlcPtr OsAlc_delete(Ptr this, AlcReq req, Ptr aux, Ptr mem) {
	auto const header = (OsAlc_Header*)((usize)mem - OsAlc_dataoffset);

	auto const map_base = header->map_base;
	auto const map_size = header->map_size;

	auto const res = linux_munmap(map_base, map_size);
	if (res < 0) switch (-res) {
		default:
			return AlcPtr_set(AlcRes_ErrInternal);
	}

	return AlcPtr_Ok;
}

AlcPtr OsAlc_invoke(Ptr this, AlcReq req, Ptr aux, Ptr mem) {
	AlcAttr constexpr attr = FLAGS(AlcAttr,
		IntentNew, IntentDelete, FeatureZero, Concurrent
	);

	switch (FIELD_GET_CAST(AlcIntent, req)) {
		default:
			return AlcPtr_set(AlcRes_ErrUnsupported);

		case AlcIntent_Attr:
			if (!aux)
				return AlcPtr_set(AlcRes_ErrInvalidAux);
			*(AlcAttr*)aux = attr;
			return AlcPtr_Ok;

		case AlcIntent_New:
			return OsAlc_new(this, req, aux);

		case AlcIntent_Delete:
			return OsAlc_delete(this, req, aux, mem);
	}
}

IAlc_GENERATE_KNOWN(OsAlc, Ptr)

#if Alc_PTRTAG
	Alc constexpr OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);
#else
	Alc const OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);
#endif
