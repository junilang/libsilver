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

AlcRes OsAlc_query(Ptr this, AlcReq req, ConstPtr hint, Ptr offers_) {
	if (req & FLAG(AlcReq, Promise))
		return AlcRes_ErrUnsupported;

	usize page_size = env_pagesz();

	switch (FIELD_GET_CAST(AlcRelative, req)) {
		default:
			return AlcRes_ErrInvalidRelative;
		case AlcRelative_None:
		case AlcRelative_Local:;
	}

	ualign align = AlcAlign_get(FIELD_GET_CAST(AlcAlign, req));

	if (align > page_size)
		return AlcRes_ErrInvalidAlign;

	if (align < alignof(OsAlc_Header))
		align = alignof(OsAlc_Header);

	u8 offers_size = FIELD_GET(AlcOffersSize, req);
	AlcOffer *offers = offers_;

	if (offers_size == 0)
		return AlcRes_Ok;


	// we know the page will be aligned to page_size so we can do a
	// straightforward calculation

	usize offset = usize_align(sizeof(OsAlc_Header), align);
	usize req_size = FIELD_GET(AlcSize, req);
	usize next_page = usize_align(req_size + offset, page_size);

	switch (FIELD_GET_CAST(AlcIntent, req)) {
		case AlcIntent_Least: {
			intent_least:;
			usize offer = next_page - offset;

			for (u8 i = 0; i < offers_size; i++) {
				AlcSize size = offer + page_size * i;
				if (size > AlcSize_max)
					break;
				offers[i] = FIELD_SETN(AlcSize, size);
			}

			return AlcRes_Ok;
		}

		case AlcIntent_Loose: {
			if (next_page == page_size) // cannot offer lower
				goto intent_least;

			usize higher = next_page - offset;
			usize lower = higher - page_size;

			if (higher > AlcSize_max) {
				offers[0] = lower;
				return AlcRes_Ok;
			}

			if ((higher - req_size) > (req_size - lower)) {
				offers[0] = lower;
				if (offers_size > 1)
					offers[1] = higher;
			} else {
				offers[0] = higher;
				if (offers_size > 1)
					offers[1] = lower;
			}

			for (AlcOffersSize i = 2; i < offers_size; i++) {
				offers[i] = higher + page_size * (i - 1);
			}

			return AlcRes_Ok;
		}

		default:
			return AlcRes_ErrUnsupported;
	}
}

Ptr OsAlc_resolve(Ptr this, AlcPromise *offers, AlcOffersSize offers_size, AlcOffersSize accept_index) {
	return AlcRes_set(AlcRes_ErrUnsupported);
}

AlcRes OsAlc_lock(Ptr this, AlcLockIntent intent) {
	return AlcRes_ErrUnsupported;
}

AlcAttr OsAlc_attr(Ptr this) {
	return FLAG(AlcAttr, ThreadSafe, FeatureRelativeLocal);
}

IAlc_GENERATE_KNOWN(OsAlc, Ptr)

#if Alc_PTRTAG
	constexpr Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#else
	const Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#endif
