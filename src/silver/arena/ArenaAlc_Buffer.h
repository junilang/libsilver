typedef struct {
	ArenaAlc_Units off_global; // distance from beginning of chunk data section to buffer data section
	ArenaAlc_Units off_size; // size of data section
	alignas(ArenaAlc_unit) char data[];
} ArenaAlc_Buffer;

static_assert(alignof(ArenaAlc_Buffer) == ArenaAlc_unit);
