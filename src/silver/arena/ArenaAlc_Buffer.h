typedef struct {
	ArenaAlc_Units off_end;
	alignas(ArenaAlc_unit) char data[];
} ArenaAlc_Buffer;

static_assert(alignof(ArenaAlc_Buffer) == ArenaAlc_unit);
