typedef struct {
	usize map_size;
	u32 req_size_offset;
	u32 base_offset;
	char data[];
} OsAlc_Header;

Ptr OsAlc_new(Ptr this, AlcReq req, ConstPtr hint) {
	const usize page_size = env_pagesz();

	switch (FIELD_GET_CAST(AlcRelative, req)) {
		default:
			return AlcRes_set(AlcRes_ErrInvalidRelative);
		case AlcRelative_None:
			hint = nullptr;
		case AlcRelative_Local:
	}

	ualign align = AlcAlign_get(FIELD_GET_CAST(AlcAlign, req));
	if (align > page_size)
		return AlcRes_set(AlcRes_ErrInvalidAlign);

	if (align < alignof(OsAlc_Header))
		align = alignof(OsAlc_Header);

	usize offset = usize_align(sizeof(OsAlc_Header), align);
	usize req_size = FIELD_GET(AlcSize, req);
	usize next_page = usize_align(req_size + offset, page_size);

	iword res = linux_mmap(hint, next_page,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0
	);

	if (res <= -1 && res >= -4095) {
		switch (-res) {
			case ENOMEM:
				return AlcRes_set(AlcRes_ErrNoMemory);
			default:
				return AlcRes_set(AlcRes_ErrInternal);
		}
	}

	usize mem_begin = (usize)res;
	usize mem = mem_begin + offset;
	OsAlc_Header *header = (OsAlc_Header*)mem - 1;

	header->map_size = next_page;
	header->req_size_offset = (u32)(next_page - req_size);
	header->base_offset = (u32)(mem - mem_begin);

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
	usize page_size = env_pagesz();

	switch (FIELD_GET_CAST(AlcRelative, req)) {
		default:
			return FIELD_SET(AlcNrs_Offer, Refuse) | FIELD_SETN(AlcNrs_RefuseReason, AlcRes_ErrInvalidRelative);
		case AlcRelative_None:
		case AlcRelative_Local:;
	}

	u8 alts_size = FIELD_GET(AlcNrq_Alts, req);

	ualign align = AlcAlign_get(FIELD_GET_CAST(AlcAlign, req));

	if (align > page_size)
		return FIELD_SET(AlcNrs_Offer, Refuse) | FIELD_SETN(AlcNrs_RefuseReason, AlcRes_ErrInvalidAlign);

	if (align < alignof(OsAlc_Header))
		align = alignof(OsAlc_Header);

	// we know the page will be aligned to page_size so we can do a
	// straightforward calculation

	usize offset = usize_align(sizeof(OsAlc_Header), align);
	usize req_size = FIELD_GET(AlcSize, req);
	usize next_page = usize_align(req_size + offset, page_size);

	switch (FIELD_GET_CAST(AlcNrq_Intent, req)) {
		case AlcNrq_Intent_Least: {
			intent_least:;
			usize offer = next_page - offset;

			// offer higher alternatives
			for (u8 i = 0; i < alts_size; i++) {
				alts[i] = offer + page_size * (i + 1);
			}

			return FIELD_SETN(AlcSize, next_page - offset) | FIELD_SETN(AlcNrs_Alts, alts_size);
		}

		case AlcNrq_Intent_Loose: {
			if (next_page == page_size) // cannot offer lower
				goto intent_least;

			usize higher = next_page - offset;
			usize lower = higher - page_size;
			usize offer;

			if ((higher - req_size) > (req_size - lower)) {
				offer = lower;
				if (alts_size)
					alts[0] = higher;
			} else {
				offer = higher;
				if (alts_size)
					alts[0] = lower;
			}

			for (u8 i = 1; i < alts_size; i++) {
				alts[i] = higher + page_size * (i + 1);
			}

			return FIELD_SETN(AlcSize, offer) | FIELD_SETN(AlcNrs_Alts, alts_size);
		}

		default:
			return FIELD_SET(AlcNrs_Offer, Refuse) | FIELD_SETN(AlcNrs_RefuseReason, AlcRes_ErrUnimplemented);
	}
}

AlcAttr OsAlc_attr(Ptr this) {
	return FLAG(AlcAttr, ThreadSafe, NoResize);
}

IAlc_GENERATE_KNOWN(OsAlc, Ptr)

#if Alc_PTRTAG
	constexpr Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#else
	const Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#endif
