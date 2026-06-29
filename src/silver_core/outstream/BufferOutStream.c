typedef struct {
	ubyte *buffer;
	u32 size;
	u32 capacity;
} BufferOutStream;

#ifndef BufferOutStream_SAFE
	#define BufferOutStream_SAFE BUILD_SAFE
#endif

OutStreamRes BufferOutStream_write(BufferOutStream *this, ConstPtr buffer, usize buffer_size) {
	auto const index = this->size;

	if (index + buffer_size > this->capacity)
		return OutStreamRes_ErrOverflow;

	memcpy(this->buffer + index, buffer, buffer_size);

	this->size = index + (u32)buffer_size;

	return OutStreamRes_Ok;
}

OutStreamRes BufferOutStream_flush(BufferOutStream *this) {
	return OutStreamRes_Ok;
}

OutStreamAttr BufferOutStream_attr(BufferOutStream *this) {
	return 0;
}

IOutStream_GENERATE_KNOWN(BufferOutStream, BufferOutStream*)
