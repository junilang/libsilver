#ifndef Vec_SAFE
	#define Vec_SAFE LIBSILVER_SAFE
#endif

typedef struct {
	ubyte *data;
	usize size;
	usize capacity;
} Vec;

ualign constexpr Vec_defaultalign = _Alignof(Ptr);
usize constexpr Vec_basecapacity = sizeof(Ptr) * 4;

bool Vec_capacityscale_(usize *capacity) {
	auto const value = *capacity;
	return chkdadd(value, value / 2, capacity);
}

#if 0
void Vec_zero(Vec *this) {
	this->data = nullptr;
	this->capacity = 0;
	this->size = 0;
}
#endif

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

	auto const data = this->data;

	AlcReq req = AlcReq_size(new_size) | AlcReq_align(alc_align);
	if (data)
		req |= FIELD(AlcIntent, Resize);
	else
		req |= FIELD(AlcIntent, New);

	usize alc_size;
	auto const ptr = Alc_invoke(alc, req, &alc_size, data);
	auto const res = AlcPtr_get(ptr);
	if (res) return res;

	this->data = (Ptr)ptr;
	this->capacity = alc_size;

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
		capacity = Vec_basecapacity;

	do {
		if (Vec_capacityscale_(&capacity))
			return AlcRes_ErrOverflow;
	} while (new_size > capacity);

	auto const data = this->data;

	AlcReq req = AlcReq_size(capacity) | AlcReq_align(alc_align);
	if (data)
		req |= FIELD(AlcIntent, Resize);
	else
		req |= FIELD(AlcIntent, New);

	usize alc_size;
	auto const ptr = Alc_invoke(alc, req, &alc_size, data);
	auto const res = AlcPtr_get(ptr);
	if (res) return res;

	this->data = (Ptr)ptr;
	this->capacity = alc_size;

	return AlcRes_Ok;
}

Ptr Vec_append_unsafe(Vec *this, usize size) {
	usize end = this->size;
	this->size = end + size;
	return this->data + end;
}

[[nodiscard]]
AlcPtr Vec_append_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize const end = this->size;
	usize new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	auto const data = this->data;

	if (new_size > this->capacity) {
		AlcReq req = AlcReq_size(new_size) | AlcReq_align(alc_align);
		if (data)
			req |= FIELD(AlcIntent, Resize);
		else
			req |= FIELD(AlcIntent, New);

		usize alc_size;
		auto const ptr = Alc_invoke(alc, req, &alc_size, data);
		if (AlcPtr_check(ptr))
			return ptr;

		this->data = (Ptr)ptr;
		this->capacity = alc_size;
	}

	this->size = new_size;
	return (AlcPtr)((usize)data + end);
}

[[nodiscard]]
AlcPtr Vec_push_aligned(Vec *this, Alc alc, usize size, ualign alc_align) {
	usize const end = this->size;
	usize new_size;
	if (chkdadd(end, size, &new_size))
		return AlcPtr_set(AlcRes_ErrOverflow);

	auto const data = this->data;

	usize capacity = this->capacity;
	if (new_size > capacity) {
		if (capacity == 0)
			capacity = Vec_basecapacity;

		do {
			if (Vec_capacityscale_(&capacity))
				return AlcPtr_set(AlcRes_ErrOverflow);
		} while (new_size > capacity);

		AlcReq req = AlcReq_size(capacity) | AlcReq_align(alc_align);
		if (data)
			req |= FIELD(AlcIntent, Resize);
		else
			req |= FIELD(AlcIntent, New);

		usize alc_size;
		auto const ptr = Alc_invoke(alc, req, &alc_size, data);
		if (AlcPtr_check(ptr))
			return ptr;

		this->data = (Ptr)ptr;
		this->capacity = alc_size;
	}

	this->size = new_size;
	return (AlcPtr)((usize)data + end);
}

[[nodiscard]]
AlcPtr Vec_append(Vec *this, Alc alc, usize size) {
	return Vec_append_aligned(this, alc, size, Vec_defaultalign);
}

[[nodiscard]]
AlcPtr Vec_push(Vec *this, Alc alc, usize size) {
	return Vec_push_aligned(this, alc, size, Vec_defaultalign);
}

[[nodiscard]]
AlcRes Vec_reserve(Vec *this, Alc alc, usize size) {
	return Vec_reserve_aligned(this, alc, size, Vec_defaultalign);
}

[[nodiscard]]
AlcRes Vec_scale(Vec *this, Alc alc, usize size) {
	return Vec_scale_aligned(this, alc, size, Vec_defaultalign);
}
