typedef struct {
	OutStream parent;
	ubyte *buffer;
	u32 size;
	u32 capacity;
} BufferedOutStream;

void BufferedOutStream_init(
	BufferedOutStream *this, OutStream parent, ubyte *buffer, u32 capacity
) {
	this->parent = parent;
	this->buffer = buffer;
	this->capacity = capacity;
	this->size = 0;
}

OutStreamRes BufferedOutStream_flush(BufferedOutStream *this) {
	if (this->size) {
		auto res = OutStream_write(this->parent, this->buffer, this->size);
		if (res) return res;
		this->size = 0;
	}

	return OutStream_flush(this->parent);
}

OutStreamRes BufferedOutStream_write(BufferedOutStream *this, ConstPtr buffer, usize buffer_size) {
	if (buffer_size >= this->capacity) {
		if (this->size) {
			auto res = OutStream_write(this->parent, this->buffer, this->size);
			if (res) return res;

			this->size = 0;
		}

		return OutStream_write(this->parent, buffer, buffer_size);
	}

	else if (buffer_size > (this->capacity - this->size)) {
		auto res = OutStream_write(this->parent, this->buffer, this->size);
		if (res) return res;

		memcpy(this->buffer, buffer, buffer_size);
		this->size = (u32)buffer_size;

		return OutStreamRes_Ok;
	}

	else {
		memcpy(this->buffer + this->size, buffer, buffer_size);
		this->size += (u32)buffer_size;

		return OutStreamRes_Ok;
	}
}

OutStreamAttr BufferedOutStream_attr(BufferedOutStream *this) {
	return 0;
}

IOutStream_GENERATE_KNOWN(BufferedOutStream, BufferedOutStream*)
