// the smallest unit of memory ArenaAlc operates on
// this allows for usage of 32 bit internal offsets
constexpr ualign ArenaAlc_unit = 8;
typedef u32 ArenaAlc_Units;

#include "ArenaAlc_Chunk.h"
#include "ArenaAlc_Buffer.h"

typedef struct {
	Alc provider;
	ArenaAlc_Chunk *head;
	ArenaAlc_Units chunk_size;
} ArenaAlc;

static_assert(alignof(ArenaAlc) == ArenaAlc_unit);

AlcRes ArenaAlc_pushchunk(ArenaAlc *this, usize least_size) {
	auto current = this->head;
	ArenaAlc_Chunk *next;
	if (current) next = current->next;
	else next = nullptr;

	if (next && (next->off_end * ArenaAlc_unit >= least_size)) {
		this->head = next;
		return AlcRes_Ok;
	}

	auto chunk = ArenaAlc_Chunk_allocate(
		this->provider, current, least_size, this->chunk_size * ArenaAlc_unit
	);
	{
		auto res = AlcRes_get(chunk);
		switch (res) {
			default:
				return res;
			case AlcRes_Ok:
		}
	}

	if (current)
		current->next = chunk;
	chunk->prev = current;
	chunk->next = next;
	this->head = chunk;

	return AlcRes_Ok;
}

AlcRes ArenaAlc_init(ArenaAlc *this, Alc provider, usize chunk_size) {
	if (!chunk_size)
		return AlcRes_ErrInvalidSize;

	this->provider = provider;
	this->chunk_size = (ArenaAlc_Units)(usize_align(chunk_size, ArenaAlc_unit) / 8);
	this->head = nullptr;

	auto res = ArenaAlc_pushchunk(this, 0);
	switch (res) {
		default:
			return res;
		case AlcRes_Ok:
	}

	return AlcRes_Ok;
}

Ptr ArenaAlc_new(ArenaAlc *this, AlcReq req, ConstPtr hint) {
	usize req_size = FIELD_GET(AlcSize, req);
	if (!req_size) return AlcRes_set(AlcRes_ErrInvalidSize);

	auto align = AlcAlign_get(FIELD_GET(AlcAlign, req));
	if (align < ArenaAlc_unit)
		align = ArenaAlc_unit;

	req_size = usize_align(req_size, align);

	auto size = (ArenaAlc_Units)(req_size / ArenaAlc_unit);

	// TODO search evicted buffers before allocating new

	bool chunk_is_new = false;

	try_again:;
	auto chunk = this->head;
	// calculate aligned address of buffer data and convert back to units
	usize ptr = (usize)&chunk->data + (chunk->off_head * ArenaAlc_unit) + sizeof(ArenaAlc_Buffer);
	ptr = usize_align(ptr, align);

	auto offset = (ArenaAlc_Units)((ptr - (usize)&chunk->data) / ArenaAlc_unit);
	auto new_head = offset + size;

	if (new_head > chunk->off_end) {
		// we already tried allocating a new chunk
		if (chunk_is_new)
			return AlcRes_set(AlcRes_ErrInternal);

		usize least_size = sizeof(ArenaAlc_Buffer) + req_size;
		// we only need to worry about alignment if the
		// requested alignment is more than a unit
		if (align > ArenaAlc_unit) {
			least_size = align + usize_align(least_size, align);
		}

		auto res = ArenaAlc_pushchunk(this, least_size);
		switch (res) {
			default:
				return AlcRes_set(res);
			case AlcRes_Ok:
		}

		chunk_is_new = true;
		goto try_again;
	}

	auto header = (ArenaAlc_Buffer*)(ptr - sizeof(ArenaAlc_Buffer));
	header->off_size = size;
	header->off_global = offset;
	chunk->off_head = new_head;

	return (Ptr)ptr;
}

Ptr ArenaAlc_resize(ArenaAlc *this, Ptr mem, AlcReq req, ConstPtr hint) {
	return AlcRes_set(AlcRes_ErrUnimplemented);
}

AlcRes ArenaAlc_delete(ArenaAlc *this, Ptr mem, AlcReq req) {
	return AlcRes_Ok;
}

AlcNrs ArenaAlc_negotiate(ArenaAlc *this, AlcReq req, ConstPtr hint, usize *alts) {
	return FIELD_SETN(AlcSize, FIELD_GET(AlcSize, req));
}

AlcAttr ArenaAlc_attr(ArenaAlc *this) {
	return 0;
}
