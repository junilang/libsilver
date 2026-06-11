typedef union {
	usize map_size;
	u32 req_size_offset;
	u16 base_offset;
} OsAlc_Header;

Ptr OsAlc_new(Ptr this, AlcReq req, ConstPtr hint) {
	const usize pagesz = env_pagesz();

	switch (FIELD_GET_CAST(AlcRelative, req)) {
		default:
			return AlcRes_set(AlcRes_ErrInvalidRelative);
		case AlcRelative_None:
			hint = nullptr;
		case AlcRelative_Local:
	}

	ualign align = AlcAlign_get(FIELD_GET(AlcAlign, req));
	if (align < alignof(OsAlc_Header)) {
		align = alignof(OsAlc_Header);
	}

	if (align > pagesz)
		return AlcRes_set(AlcRes_ErrUnsupported);

	usize req_size = FIELD_GET(AlcSize, req);

	// make sure memory can contain header and is aligned to requested alignment
	usize map_size = sizeof(OsAlc_Header);
	if (map_size < align) {
		map_size = align;
	} else {
		map_size = (map_size + (align - 1)) & (~((usize)align - 1));
	}

	map_size += req_size;

	// ceil to multiple of page size (which is always power of 2)
	map_size = (map_size + (pagesz - 1)) & (~(pagesz - 1));

	iword res = linux_mmap(hint, map_size,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0
	);

	if (res <= -1 && res >= -4095)
		return AlcRes_set(AlcRes_ErrInternal);

	usize mem_begin = (usize)res;

	usize mem = (usize)mem_begin + sizeof(OsAlc_Header);
	mem = (mem + (align - 1)) & (~((usize)align - 1));

	OsAlc_Header *header = (OsAlc_Header*)mem - 1;
	header->map_size = map_size;
	header->req_size_offset = (u32)(map_size - req_size);
	header->base_offset = (u16)(mem - (usize)mem_begin);

	return (Ptr)mem;
}

Ptr OsAlc_resize(Ptr this, Ptr mem, AlcReq req, ConstPtr hint) {
	return AlcRes_set(AlcRes_ErrUnsupported);
}

AlcRes OsAlc_delete(Ptr this, Ptr mem, AlcReq req) {
	OsAlc_Header *header = (OsAlc_Header*)mem - 1;
	Ptr base = (Ptr)((usize)mem - header->base_offset);

	iword res = linux_munmap(base, header->map_size);
	if (res < 0)
		return AlcRes_ErrInternal;

	return AlcRes_Ok;
}

AlcNrs OsAlc_negotiate(Ptr this, AlcNrq req, ConstPtr hint, usize *alts) {
	// TODO
	usize page_size = env_pagesz();

	auto intent = FIELD_GET(AlcNrq_Intent, req);
	usize req_size = FIELD_GET(AlcSize, req);

	switch (intent) {
		case AlcNrq_Intent_Least: {
			// if user needs at least this much memory,
			// we suggest the next multiple of the page size
		}
	}

	return FIELD_SETN(AlcSize, FIELD_GET(AlcSize, req));
}

AlcAttr OsAlc_attr(Ptr this) {
	return 0;
}

IAlc_GENERATE_KNOWN(OsAlc, Ptr)

#if Alc_PTRTAG
	constexpr Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#else
	const Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#endif
