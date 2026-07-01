typedef struct {
	usize map_size;
	usize base_offset;
	char data[];
} OsAlc_Header;

[[nodiscard, gnu::malloc]]
AlcPtr OsAlc_new(AlcReq *req) {
	auto const page_size = env_pagesz();

	ConstPtr hint;
	switch (req->relative) {
		default:
			return AlcPtr_set(AlcRes_ErrInvalidRelative);
		case AlcRelative_None:
			hint = nullptr;
			break;
		case AlcRelative_Local:
			hint = req->hint;
			break;
	}

	ualign align = req->align;
	if (align > page_size)
		return AlcPtr_set(AlcRes_ErrInvalidAlign);

	if (align < alignof(OsAlc_Header))
		align = alignof(OsAlc_Header);

	usize offset = usize_align(sizeof(OsAlc_Header), align);

	// round to page size
	usize map_size = usize_align(req->size + offset, page_size);

	iword res = linux_mmap(hint, map_size,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0
	);

	if (res <= -1 && res >= -4095) {
		switch (-res) {
			case ENOMEM:
				return AlcPtr_set(AlcRes_ErrNoMemory);
			default:
				return AlcPtr_set(AlcRes_ErrInternal);
		}
	}

	auto mem_begin = (usize)res;
	auto mem = mem_begin + offset;
	auto header = (OsAlc_Header*)mem - 1;

	header->map_size = map_size;
	header->base_offset = mem - mem_begin;

	req->size = map_size - offset;

	return (Ptr)mem;
}

AlcRes OsAlc_delete(Ptr mem) {
	auto header = (OsAlc_Header*)mem - 1;
	auto base = (Ptr)((usize)mem - header->base_offset);

	auto res = linux_munmap(base, header->map_size);
	if (res < 0)
		return AlcRes_ErrInternal;

	return AlcRes_Ok;
}

AlcRes OsAlc_query(AlcReq *req, usize *offers) {
	auto const page_size = env_pagesz();

	switch (req->relative) {
		default: return AlcRes_ErrInvalidRelative;
		case AlcRelative_None:
		case AlcRelative_Local:
	}

	ualign align = req->align;

	if (align > page_size)
		return AlcRes_ErrInvalidAlign;

	if (align < alignof(OsAlc_Header))
		align = alignof(OsAlc_Header);

	u8 offers_size = req->offers_size;
	if (!offers_size)
		return AlcRes_Ok;

	usize offset = usize_align(sizeof(OsAlc_Header), align);
	usize req_size = req->size;
	usize next_page = usize_align(req_size + offset, page_size);

	switch (req->range) {
		default: return AlcRes_ErrInvalidRange;

		case AlcRange_Least: {
			range_least:;
			usize offer = next_page - offset;
			for (u8 i = 0; i < offers_size; i++) {
				offers[i] = offer + page_size * i;
			}

			return AlcRes_Ok;
		}

		case AlcRange_Loose: {
			// cannot offer lower
			if (next_page == page_size) goto range_least;

			usize higher = next_page - offset;
			usize lower = higher - page_size;

			if ((higher - req_size) > (req_size - lower)) {
				offers[0] = lower;
				if (offers_size > 1)
					offers[1] = higher;
			} else {
				offers[0] = higher;
				if (offers_size > 1)
					offers[1] = lower;
			}

			for (u8 i = 2; i < offers_size; i++) {
				offers[i] = higher + page_size * (i - 1);
			}

			return AlcRes_Ok;
		}
	}
}

constexpr AlcAttr OsAlc_attr = FLAGS(AlcAttr,
	IntentNew, IntentDelete, IntentQuery,
	FeatureRelativeLocal, FeatureZero, ThreadSafe,
);

AlcPtr OsAlc_invoke(Ptr this, AlcReq *req, Ptr arg, Ptr mem) {
	switch (req->intent) {
		default: return AlcPtr_set(AlcRes_ErrUnsupported);

		case AlcIntent_Attr: return (AlcPtr)(usize)OsAlc_attr;
		case AlcIntent_New: return OsAlc_new(req);
		case AlcIntent_Delete: return AlcPtr_set(OsAlc_delete(mem));
		case AlcIntent_Query: return AlcPtr_set(OsAlc_query(req, arg));
	}
}

IAlc_GENERATE_KNOWN(OsAlc, Ptr)

#if Alc_PTRTAG
	constexpr Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#else
	const Alc OsAlc = CONSTEXPR_Alc_upcast(OsAlc, 0);

#endif
