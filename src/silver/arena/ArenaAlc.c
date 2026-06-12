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

AlcRes ArenaAlc_init(ArenaAlc *this, Alc provider, usize chunk_size) {
	this->provider = provider;

	chunk_size = (chunk_size + (ArenaAlc_unit - 1)) & (~((usize)ArenaAlc_unit - 1));
	this->chunk_size = (ArenaAlc_Units)(chunk_size / 8);

	ArenaAlc_Chunk *head = ArenaAlc_Chunk_allocate(
		provider, nullptr,
		// chunk size is still in bytes here
		chunk_size / 2, chunk_size
	);

	switch (AlcRes_get(head)) {
		default:
			return AlcRes_get(head);
		case AlcRes_Ok:
	}

	this->head = head;

	return AlcRes_Ok;
}

Ptr ArenaAlc_new(ArenaAlc *this, AlcReq req, ConstPtr hint) {
	usize size = FIELD_GET(AlcSize, req);

	ualign align = AlcAlign_get(FIELD_GET(AlcAlign, req));
	if (align < ArenaAlc_unit)
		align = ArenaAlc_unit;

	// ceil size to alignment multiple
	size = (size + (align - 1)) & (~((usize)align - 1));

	ArenaAlc_Chunk *chunk = this->head;

	// get searchable address
	usize ptr = (usize)&chunk->data + (chunk->off_head * ArenaAlc_unit) + sizeof(ArenaAlc_Buffer);
	// ceil address to alignment multiple
	ptr = (ptr + (align - 1)) & (~((usize)align - 1));

	usize end = ptr + size;
	auto new_head = (ArenaAlc_Units)((end - (usize)&chunk->data) / ArenaAlc_unit);

	// buffer would overflow
	if (new_head > chunk->off_end)
		return AlcRes_set(AlcRes_ErrNoMemory);

	auto header = (ArenaAlc_Buffer*)(ptr - sizeof(ArenaAlc_Buffer));
	header->off_end = (ArenaAlc_Units)(size / ArenaAlc_unit);
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
