typedef struct {
	OutStream parent;
	u32 size;
	u32 capacity;
	ubyte data[];
} BufferedOutStream;

void BufferedOutStream_init(BufferedOutStream *this, OutStream parent, u32 capacity) {
}
