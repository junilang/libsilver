typedef struct {
	u32 size; // max 4GB
	ubyte data[];
} StaticAlc_Header;

constexpr ualign StaticAlc_minalign = alignof(StaticAlc_Header);

typedef struct {
	u32 size;
	u32 head;
	alignas(StaticAlc_minalign) ubyte data[];
} StaticAlc;

Ptr StaticAlc_new(StaticAlc *this, AlcReq *req) {
	switch (req->relative) {
		default: return AlcRes_set(AlcRes_ErrInvalidRelative);

		case AlcRelative_None:
		case AlcRelative_Local:
	}

	ualign align = req->align;
	if (align < StaticAlc_minalign)
		align = StaticAlc_minalign;

	usize alc_size = usize_align(sizeof(StaticAlc_Header) + req->size, align);


}

Ptr StaticAlc_resize(StaticAlc *this, AlcReq *req, Ptr mem) {
	return AlcRes_set(AlcRes_ErrInternal);
}

constexpr AlcAttr StaticAlc_attr = FLAG(AlcAttr,
	IntentNew, IntentResize, IntentDelete, IntentQuery, FeatureZero, FeatureRelativeLocal
);

Ptr StaticAlc_invoke(StaticAlc *this, AlcReq *req, Ptr arg, Ptr mem) {
	switch (req->intent) {
		default: return AlcRes_set(AlcRes_ErrUnsupported);

		case AlcIntent_Attr: return (Ptr)(usize)StaticAlc_attr;
		case AlcIntent_New: return StaticAlc_new(this, req);
		case AlcIntent_Resize: return StaticAlc_resize(this, req, mem);
		case AlcIntent_Delete: return AlcRes_set(AlcRes_Ok);
	}
}

IAlc_GENERATE_KNOWN(StaticAlc, StaticAlc*)
