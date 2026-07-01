typedef struct {
	u32 size; // max 4GB
	ubyte data[];
} StaticAlc_Buffer;

constexpr ualign StaticAlc_minalign = alignof(StaticAlc_Buffer);
constexpr auto StaticAlc_Buffer_dataoffset = __builtin_offsetof(StaticAlc_Buffer, data);

typedef struct {
	u32 end;
	u32 head;
	alignas(StaticAlc_minalign) ubyte data[];
} StaticAlc;

[[nodiscard, gnu::malloc]]
AlcPtr StaticAlc_new(StaticAlc *this, AlcReq *req) {
	switch (req->relative) {
		default: return AlcPtr_set(AlcRes_ErrInvalidRelative);

		case AlcRelative_None:
		case AlcRelative_Local:
	}

	ualign align = req->align;
	if (align < StaticAlc_minalign)
		align = StaticAlc_minalign;

	usize size = usize_align(req->size, align);

	if (size > u32_max)
		return AlcPtr_set(AlcRes_ErrInvalidSize);

	usize ptr = (usize)this->data + this->head;
	ptr = usize_align(ptr + StaticAlc_Buffer_dataoffset, align);

	u32 new_head = (u32)((ptr + size) - (usize)this->data);

	if (new_head > this->end)
		return AlcPtr_set(AlcRes_ErrNoMemory);

	auto buffer = (StaticAlc_Buffer*)(ptr - StaticAlc_Buffer_dataoffset);

	buffer->size = (u32)size;

	this->head = new_head;

	if (req->flags & FLAG(AlcFlag_Zero))
		memset((Ptr)ptr, 0, size);

	return (Ptr)ptr;
}

[[nodiscard, gnu::malloc]]
AlcPtr StaticAlc_resize(StaticAlc *this, AlcReq *req, Ptr mem) {
	if (!mem) return AlcPtr_set(AlcRes_ErrInvalidMem);

	switch (req->relative) {
		default: return AlcPtr_set(AlcRes_ErrInvalidRelative);

		case AlcRelative_None:
		case AlcRelative_Local:
	}

	ualign align = req->align;
	if (align < StaticAlc_minalign)
		align = StaticAlc_minalign;

	usize size = usize_align(req->size, align);

	if (size > u32_max)
		return AlcPtr_set(AlcRes_ErrInvalidSize);

	auto buffer = (StaticAlc_Buffer*)((usize)mem - StaticAlc_Buffer_dataoffset);
	auto const old_size = buffer->size;

	// if size does not need to increase and alignment matches, do nothing
	if ((size <= old_size) && (((usize)mem & align) == 0))
		return mem;

	usize ptr = (usize)this->data + this->head;
	ptr = usize_align(ptr + StaticAlc_Buffer_dataoffset, align);

	u32 new_head = (u32)((ptr + size) - (usize)this->data);

	if (new_head > this->end)
		return AlcPtr_set(AlcRes_ErrNoMemory);

	auto new_buffer = (StaticAlc_Buffer*)(ptr - StaticAlc_Buffer_dataoffset);
	new_buffer->size = (u32)size;

	memcpy((Ptr)ptr, mem, old_size);
	if (req->flags & FLAG(AlcFlag_Zero))
		memset((Ptr)(ptr + old_size), 0, size - old_size);

	this->head = new_head;

	return (Ptr)ptr;
}

AlcRes StaticAlc_query(AlcReq *req, usize *offers) {
	switch (req->relative) {
		default: return AlcRes_ErrInvalidRelative;
		case AlcRelative_None:
		case AlcRelative_Local:
	}

	ualign align = req->align;
	if (align < StaticAlc_minalign)
		align = StaticAlc_minalign;

	usize size = usize_align(req->size, align);

	if (req->offers_size) {
		offers[0] = size;
		req->offers_size = 1;
	}

	return AlcRes_Ok;
}

constexpr AlcAttr StaticAlc_attr = FLAGS(AlcAttr,
	IntentNew, IntentResize, IntentDelete, IntentQuery, FeatureZero, FeatureRelativeLocal
);

AlcPtr StaticAlc_invoke(StaticAlc *this, AlcReq *req, Ptr arg, Ptr mem) {
	switch (req->intent) {
		default: return AlcPtr_set(AlcRes_ErrUnsupported);

		case AlcIntent_Attr: return (Ptr)(usize)StaticAlc_attr;
		case AlcIntent_New: return StaticAlc_new(this, req);
		case AlcIntent_Resize: return StaticAlc_resize(this, req, mem);
		case AlcIntent_Query: return AlcPtr_set(StaticAlc_query(req, arg));
		case AlcIntent_Delete: return AlcPtr_set(AlcRes_Ok);
	}
}

IAlc_GENERATE_KNOWN(StaticAlc, StaticAlc*)

StaticAlc *StaticAlc_init(Ptr mem, usize size) {
	usize end = ((usize)mem + size) & (~(usize)StaticAlc_minalign);
	usize ptr = usize_align((usize)mem, StaticAlc_minalign);

	end -= ptr;

	if (end > u32_max)
		PANIC("overflow");

	auto this = (StaticAlc*)ptr;
	this->head = 0;
	this->end = (u32)end;

	return this;
}

void StaticAlc_clear(StaticAlc *this) {
	this->head = 0;
}
