typedef struct {
	ubyte *data;
	usize size;
	usize capacity;
} Vec;

constexpr usize Vec_base_capacity = sizeof(Ptr) * 4;
constexpr ualign Vec_default_align = Alc_default_align;

#ifndef Vec_SAFE
	#define Vec_SAFE BUILD_SAFE
#endif

static usize Vec_ZZcapacityscale(usize capacity) {
	return capacity / 2;
}

void Vec_zero(Vec *this) {
	this->data = nullptr;
	this->capacity = 0;
	this->size = 0;
}

void Vec_clear(Vec *this) {
	this->size = 0;
}

[[nodiscard]]
AlcRes Vec_destroy(Vec *this, Alc alc) {
	AlcRes res;
	if (this->data)
		res = Alc_delete(alc, this->data);
	else {
		res = AlcRes_Ok;
	}

	#if Vec_SAFE
		this->capacity = 0;
		this->size = 0;
		this->data = nullptr;
	#endif

	return res;
}

Ptr Vec_begin(Vec *this) {
	return this->data;
}

Ptr Vec_end(Vec *this) {
	return this->data + this->size;
}

usize Vec_size(Vec *this) {
	return this->size;
}

usize Vec_count(Vec *this, usize elem_size) {
	return this->size / elem_size;
}

[[nodiscard]]
AlcRes Vec_reserve_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize new_size;
	if (chkdadd(this->size, size, &new_size))
		return AlcRes_ErrOverflow;

	if (new_size <= this->capacity)
		return AlcRes_Ok;

	Ptr data = this->data;

	AlcReq req = {
		.size = new_size,
		.align = alc_align,
		#if BUILD_DEBUG
			.flags = FLAG(AlcFlag_Zero),
		#endif
	};

	if (this->data)
		req.intent = AlcIntent_Resize;
	else
		req.intent = AlcIntent_New;

	data = Alc_invoke(alc, &req, nullptr, data);
	auto res = AlcPtr_get(data);
	if (res) return res;

	this->data = data;
	this->capacity = req.size;

	return AlcRes_Ok;
}

[[nodiscard]]
AlcRes Vec_scale_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize new_size;
	if (chkdadd(this->size, size, &new_size))
		return AlcRes_ErrOverflow;

	usize capacity = this->capacity;
	if (new_size <= capacity)
		return AlcRes_Ok;

	if (capacity == 0)
		capacity = Vec_base_capacity;

	do {
		if (chkdadd(capacity, Vec_ZZcapacityscale(capacity), &capacity))
			return AlcRes_ErrOverflow;
	} while (new_size > capacity);

	AlcReq req = {
		.size = capacity,
		.align = alc_align,
		#if BUILD_DEBUG
			.flags = FLAG(AlcFlag_Zero),
		#endif
	};

	Ptr data = this->data;

	if (data)
		req.intent = AlcIntent_Resize;
	else
		req.intent = AlcIntent_New;

	data = Alc_invoke(alc, &req, nullptr, data);
	auto res = AlcPtr_get(data);
	if (res) return res;

	this->data = data;
	this->capacity = req.size;

	return AlcRes_Ok;
}

Ptr Vec_append_unsafe(Vec *this, usize size) {
	usize end = this->size;
	this->size = end + size;
	return this->data + end;
}

[[nodiscard]]
AlcPtr Vec_append_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize end = this->size;
	usize new_size;
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

		this->data = data;
		this->capacity = req.size;
	}

	this->size = new_size;
	return (AlcPtr)((ubyte*)data + end);
}

[[nodiscard]]
AlcPtr Vec_push_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize end = this->size;
	usize new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	Ptr data = this->data;

	usize capacity = this->capacity;
	if (new_size > capacity) {
		if (capacity == 0)
			capacity = Vec_base_capacity;

		do {
			if (chkdadd(capacity, Vec_ZZcapacityscale(capacity), &capacity))
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

		this->data = data;
		this->capacity = req.size;
	}

	this->size = new_size;
	return (AlcPtr)((ubyte*)data + end);
}

[[nodiscard]]
AlcPtr Vec_append(Vec *this, Alc alc, usize size) {
	return Vec_append_aligned(this, alc, size, Vec_default_align);
}

[[nodiscard]]
AlcPtr Vec_push(Vec *this, Alc alc, usize size) {
	return Vec_push_aligned(this, alc, size, Vec_default_align);
}

[[nodiscard]]
AlcRes Vec_reserve(Vec *this, Alc alc, usize size) {
	return Vec_reserve_aligned(this, alc, size, Vec_default_align);
}

[[nodiscard]]
AlcRes Vec_scale(Vec *this, Alc alc, usize size) {
	return Vec_scale_aligned(this, alc, size, Vec_default_align);
}
