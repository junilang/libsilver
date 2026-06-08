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

	memcopy(this->buffer + index, buffer, buffer_size);

	this->size = index + (u32)buffer_size;
}

void BufferOutStream_flush(BufferOutStream *this) {}

IOutStream_GENERATE_KNOWN(BufferOutStream, BufferOutStream*)
