typedef struct {
	ubyte *data;
	u32 size;
	u32 capacity;
} Vec32;

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
	return this->size / elem_size;
}

[[nodiscard]]
AlcPtr Vec32_append_aligned(Vec32 *this, Alc alc, u32 size, ualign alc_align) {
	u32 end = this->size;
	u32 new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	auto data = this->data;

	if (new_size > this->capacity) {
		AlcReq req = {
			.size = new_size,
			.align = alc_align,
		};

		if (data)
			req.intent = AlcIntent_Resize;
		else
			req.intent = AlcIntent_New;

		AlcPtr res = Alc_invoke(alc, &req, nullptr, data);
		if (AlcPtr_get(res))
			return res;

		data = (Ptr)res;
		this->data = data;

		if (req.size > u32_max)
			return AlcPtr_set(AlcRes_ErrOverflow);

		this->capacity = (u32)req.size;
	}

	this->size = new_size;
	return (AlcPtr)(data + end);
}

[[nodiscard]]
AlcPtr Vec32_push_aligned(Vec32 *this, Alc alc, u32 size, ualign alc_align) {
	u32 end = this->size;
	u32 new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	auto data = this->data;

	u32 capacity = this->capacity;
	if (new_size > capacity) {
		if (capacity == 0)
			capacity = sizeof(Ptr) * 4;

		do {
			if (chkdadd(capacity, capacity / 2, &capacity))
				return AlcPtr_set(AlcRes_ErrOverflow);
		} while (new_size > capacity);

		AlcReq req = {
			.size = capacity,
			.align = alc_align,
		};

		if (data)
			req.intent = AlcIntent_Resize;
		else
			req.intent = AlcIntent_New;

		AlcPtr res = Alc_invoke(alc, &req, nullptr, data);
		if (AlcPtr_check(res))
			return res;

		data = (Ptr)res;
		this->data = data;

		if (req.size > u32_max)
			return AlcPtr_set(AlcRes_ErrOverflow);

		this->capacity = (u32)req.size;
	}

	this->size = new_size;
	return (AlcPtr)(data + end);
}

[[nodiscard]]
AlcPtr Vec32_append(Vec32 *this, Alc alc, u32 size) {
	return Vec32_append_aligned(this, alc, size, Alc_default_align);
}

[[nodiscard]]
AlcPtr Vec32_push(Vec32 *this, Alc alc, u32 size) {
	return Vec32_push_aligned(this, alc, size, Alc_default_align);
}
