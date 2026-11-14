typedef struct {
	usize size;
	ubyte data[];
} BumpAlc_Buffer;

auto constexpr BumpAlc_minalign = _Alignof(usize);
auto constexpr BumpAlc_dataoffset = __builtin_offsetof(BumpAlc_Buffer, data);

typedef struct {
	ubyte *head;
	ubyte *end;
} BumpAlc;

[[nodiscard]]
AlcPtr BumpAlc_new(BumpAlc *this, AlcReq req, Ptr aux) {
	auto align = AlcReq_getalign(req);
	if (align < BumpAlc_minalign)
		align = BumpAlc_minalign;

	auto const size = usize_align(FIELD_GET(AlcSize, req), align);

	auto const head = (usize)this->head;
	auto const ptr = usize_align(head + BumpAlc_dataoffset, align);
	auto const new_head = ptr + size;

	if (new_head > (usize)this->end)
		return AlcPtr_set(AlcRes_ErrNoMemory);

	auto const buffer = (BumpAlc_Buffer*)(ptr - BumpAlc_dataoffset);
	buffer->size = size;

	this->head = (ubyte*)new_head;

	if (req & FLAG(AlcFlag_Zero))
		memset((Ptr)ptr, 0, size);

	if (aux)
		*(usize*)aux = size;

	return (AlcPtr)ptr;
}

[[nodiscard]]
AlcPtr BumpAlc_resize(BumpAlc *this, AlcReq req, Ptr aux, Ptr mem) {
	auto align = AlcReq_getalign(req);
	if (align < BumpAlc_minalign)
		align = BumpAlc_minalign;

	auto const size = usize_align(FIELD_GET(AlcSize, req), align);

	auto const old_buffer = (BumpAlc_Buffer*)((usize)mem - BumpAlc_dataoffset);
	auto const old_size = old_buffer->size;

	usize copy_size;
	if (size <= old_size) {
		// return original memory if alignment
		// matches and size did not increase
		if (!((usize)mem & (align - 1)))
			return mem;

		copy_size = size;
	} else {
		copy_size = old_size;
	}

	auto const head = (usize)this->head;
	auto const ptr = usize_align(head + BumpAlc_dataoffset, align);
	auto const new_head = ptr + size;

	if (new_head > (usize)this->end)
		return AlcPtr_set(AlcRes_ErrNoMemory);

	auto const buffer = (BumpAlc_Buffer*)(ptr - BumpAlc_dataoffset);
	buffer->size = size;

	this->head = (ubyte*)new_head;

	memcpy((Ptr)ptr, mem, copy_size);

	if (copy_size < size && (req & FLAG(AlcFlag_Zero)))
		memset((Ptr)(ptr + copy_size), 0, (size - copy_size));

	if (aux)
		*(usize*)aux = size;

	return (AlcPtr)ptr;
}

[[nodiscard]]
AlcPtr BumpAlc_invoke(BumpAlc *this, AlcReq req, Ptr aux, Ptr mem) {
	AlcAttr attr = FLAGS(AlcAttr,
		IntentNew, IntentResize, IntentDelete, FeatureZero
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
			return BumpAlc_new(this, req, aux);

		case AlcIntent_Resize:
			return BumpAlc_resize(this, req, aux, mem);

		case AlcIntent_Delete:
			return AlcPtr_Ok;
	}
}

void BumpAlc_init(BumpAlc *this, ubyte *buf, usize buf_size) {
	this->head = buf;
	this->end = buf + buf_size;
}

ubyte *BumpAlc_save(BumpAlc *this) {
	return this->head;
}

void BumpAlc_restore(BumpAlc *this, ubyte *head) {
	this->head = head;
}

IAlc_GENERATE_KNOWN(BumpAlc, BumpAlc*)
