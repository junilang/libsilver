typedef struct {
	ubyte *head;
	ubyte *end;
} StaticAlc;

constexpr ualign StaticAlc_minalign = 0;

AlcAttr StaticAlc_attr(StaticAlc *this) {
	return FLAG(AlcAttr, NoResize);
}

Ptr StaticAlc_new(StaticAlc *this, AlcReq req, ConstPtr hint) {
	ualign align = AlcAlign_get(FIELD_GET_CAST(AlcAlign, req));
	if (align < StaticAlc_minalign) {
		align = StaticAlc_minalign;
	}

	usize head = (usize)this->head;

	// align buffer start
	head = (head + (align - 1)) & (~((usize)align - 1));

	usize req_size = FIELD_GET(AlcSize, req);

	auto new_head = (ubyte*)(head + req_size);
	if (new_head > this->end)
		return AlcRes_set(AlcRes_ErrNoMemory);

	return (Ptr)head;
}

Ptr StaticAlc_resize(StaticAlc *this, Ptr mem, AlcReq req, ConstPtr hint) {
	return AlcRes_set(AlcRes_ErrUnsupported);
}

AlcRes StaticAlc_delete(StaticAlc *this, Ptr mem, AlcReq req) {
	return AlcRes_Ok;
}

AlcNrs StaticAlc_negotiate(StaticAlc *this, AlcNrq req, ConstPtr hint, usize *alts) {
	// TODO
	return FIELD_SETN(AlcSize, FIELD_GET(AlcSize, req));
}

IAlc_GENERATE_KNOWN(StaticAlc, StaticAlc*)
