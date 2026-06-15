typedef struct {
	ArenaAlc_Units off_chunk; // distance from chunk pointer buffer pointer
	ArenaAlc_Units off_size; // distance from buffer pointer to end of data
	alignas(ArenaAlc_unit) char data[];
} ArenaAlc_Buffer;

static_assert(alignof(ArenaAlc_Buffer) == ArenaAlc_unit);
