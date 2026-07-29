typedef struct {
	ubyte *data;
	u32 size;
	u32 capacity;
} Vec32;

u32 Vec32_ZZcapacityscale(u32 capacity) {
	return capacity / 2;
}

void Vec32_zero(Vec32 *this) {
	this->data = nullptr;
	this->capacity = 0;
	this->size = 0;
}

void Vec32_clear(Vec32 *this) {
	this->size = 0;
}

[[nodiscard]]
AlcRes Vec32_destroy(Vec32 *this, Alc alc) {
	AlcRes res;
	if (this->data)
		res = Alc_delete(alc, this->data);
	else
		res = AlcRes_Ok;

	this->capacity = 0;
	this->size = 0;
	this->data = nullptr;

	return res;
}

Ptr Vec32_begin(Vec32 *this) {
	return this->data;
}

Ptr Vec32_end(Vec32 *this) {
	return this->data + this->size;
}

u32 Vec32_size(Vec32 *this) {
	return this->size;
}

u32 Vec32_count(Vec32 *this, usize elem_size) {
	return (u32)(this->size / elem_size);
}

[[nodiscard]]
AlcRes Vec32_reserve_aligned(Vec32 *this, Alc alc, u32 size, ualign alc_align) {
	return AlcRes_ErrUnimplemented;
}

[[nodiscard]]
AlcRes Vec32_scale_aligned(Vec32 *this, Alc alc, u32 size, ualign alc_align) {
	return AlcRes_ErrUnimplemented;
}

Ptr Vec32_append_unsafe(Vec32 *this, u32 size) {
	u32 end = this->size;
	this->size = end + size;
	return this->data + end;
}

[[nodiscard]]
AlcPtr Vec32_append_aligned(Vec32 *this, Alc alc, u32 size, ualign alc_align) {
	u32 end = this->size;
	u32 new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	Ptr data = this->data;

	if (new_size > this->capacity) {
		AlcReq req = {
			.size = new_size,
			.align = alc_align,
			#if BUILD_DEBUG
				.flags = FLAG(AlcFlag_Zero),
			#endif
		};

		if (data)
			req.intent = AlcIntent_Resize;
		else
			req.intent = AlcIntent_New;

		data = Alc_invoke(alc, &req, nullptr, data);
		if (AlcPtr_check(data))
			return data;

		if (req.size > u32_max)
			return AlcPtr_set(AlcRes_ErrOverflow);

		this->data = data;
		this->capacity = (u32)req.size;
	}

	this->size = new_size;
	return (AlcPtr)((ubyte*)data + end);
}

[[nodiscard]]
AlcPtr Vec32_push_aligned(Vec32 *this, Alc alc, u32 size, ualign alc_align) {
	u32 end = this->size;
	u32 new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	Ptr data = this->data;

	u32 capacity = this->capacity;
	if (new_size > capacity) {
		if (capacity == 0)
			capacity = Vec_base_capacity;

		do {
			if (chkdadd(capacity, Vec32_ZZcapacityscale(capacity), &capacity))
				return AlcPtr_set(AlcRes_ErrOverflow);
		} while (new_size > capacity);

		AlcReq req = {
			.size = capacity,
			.align = alc_align,
			#if BUILD_DEBUG
				.flags = FLAG(AlcFlag_Zero),
			#endif
		};

		if (data)
			req.intent = AlcIntent_Resize;
		else
			req.intent = AlcIntent_New;

		data = Alc_invoke(alc, &req, nullptr, data);
		if (AlcPtr_check(data))
			return data;

		if (req.size > u32_max)
			return AlcPtr_set(AlcRes_ErrOverflow);

		this->data = data;
		this->capacity = (u32)req.size;
	}

	this->size = new_size;
	return (AlcPtr)((ubyte*)data + end);
}

[[nodiscard]]
AlcPtr Vec32_append(Vec32 *this, Alc alc, u32 size) {
	return Vec32_append_aligned(this, alc, size, Vec_default_align);
}

[[nodiscard]]
AlcPtr Vec32_push(Vec32 *this, Alc alc, u32 size) {
	return Vec32_push_aligned(this, alc, size, Vec_default_align);
}
