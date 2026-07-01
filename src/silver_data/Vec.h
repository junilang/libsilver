typedef struct {
	ubyte *data;
	usize size;
	usize capacity;
} Vec;

void Vec_init(Vec *this) {
	this->data = nullptr;
	this->capacity = 0;
	this->size = 0;
}

void Vec_clear(Vec *this) {
	this->size = 0;
}

void Vec_destroy(Vec *this) {

}

Ptr Vec_begin(Vec *this) {
	return this->data;
}

Ptr Vec_end(Vec *this) {
	return (ubyte*)this->data + this->size;
}

usize Vec_size(Vec *this) {
	return this->size;
}

[[nodiscard]]
AlcPtr Vec_append_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize end = this->size;
	usize new_size;
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
		this->capacity = req.size;
	}

	this->size = new_size;
	return (AlcPtr)(data + end);
}

[[nodiscard]]
AlcPtr Vec_push_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize end = this->size;
	usize new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	auto data = this->data;

	usize capacity = this->capacity;
	if (new_size > capacity) {
		if (capacity == 0)
			capacity = sizeof(Ptr) * 4;

		do {
			if (chkdadd(capacity, capacity / 2, &capacity))
				return AlcPtr_set(AlcRes_ErrOverflow);
		} while (new_size > capacity);

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
		this->capacity = req.size;
	}

	this->size = new_size;
	return (AlcPtr)(data + end);
}

[[nodiscard]]
AlcPtr Vec_append(Vec *this, Alc alc, usize size) {
	return Vec_append_aligned(this, alc, size, Alc_default_align);
}

[[nodiscard]]
AlcPtr Vec_push(Vec *this, Alc alc, usize size) {
	return Vec_push_aligned(this, alc, size, Alc_default_align);
}
