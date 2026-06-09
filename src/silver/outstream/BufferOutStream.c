typedef struct {
	ubyte *buffer;
	u32 size;
	u32 capacity;
} BufferOutStream;

#ifndef BufferOutStream_SAFE
	#define BufferOutStream_SAFE BUILD_SAFE
#endif

void BufferOutStream_write(BufferOutStream *this, ConstPtr buffer, usize buffer_size) {
	auto const index = this->size;

	#if BufferOutStream_SAFE
		if (index + buffer_size > this->capacity) {
			PANIC("BufferOutStream overflow");
		}
	#endif

	memcpy(this->buffer + index, buffer, buffer_size);

	this->size = index + (u32)buffer_size;
}

void BufferOutStream_flush(BufferOutStream *this) {}

OutStreamAttr BufferOutStream_attr(BufferOutStream *this) {
	return 0;
}

IOutStream_GENERATE_KNOWN(BufferOutStream, BufferOutStream*)
