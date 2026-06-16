// the smallest unit of memory ArenaAlc operates on
// this allows for usage of 32 bit internal offsets
typedef u64 ArenaAlc_Unit;
constexpr u8 ArenaAlc_Unit_width = 64;
constexpr ualign ArenaAlc_Unit_size = sizeof(ArenaAlc_Unit);

typedef u32 ArenaAlc_Units;

#include "ArenaAlc_Buffer.h"
#include "ArenaAlc_Chunk.h"

typedef struct {
	Alc provider;
	ArenaAlc_Chunk *head;
	ArenaAlc_Units chunk_size;
} ArenaAlc;

static_assert(alignof(ArenaAlc) == ArenaAlc_Unit_size);
