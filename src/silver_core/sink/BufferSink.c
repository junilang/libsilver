#ifndef BufferSink_SAFE
	#define BufferSink_SAFE LIBSILVER_SAFE
#endif

typedef struct {
	ubyte *buffer;
	u32 size;
	u32 capacity;
} BufferSink;

void BufferSink_init(BufferSink *this, ubyte *buffer, u32 capacity) {
	this->buffer = buffer;
	this->size = 0;
	this->capacity = capacity;
}

String BufferSink_asString(BufferSink *this) {
	return String_create(this->buffer, this->size);
}

SinkRes BufferSink_invoke(BufferSink *this, SinkReq req, Ptr aux, ConstPtr src) {
	constexpr SinkAttr attr = FLAGS(SinkAttr,
		IntentWrite, IntentFlush
	);

	switch (FIELD_GET_CAST(SinkIntent, req)) {
		default:
			return SinkRes_ErrUnsupported;

		case SinkIntent_Attr:
			*(SinkAttr*)aux = attr;
			return SinkRes_Ok;

		case SinkIntent_Flush:
			return SinkRes_Ok;

		case SinkIntent_Write:
	}

	usize const req_size = FIELD_GET(SinkSize, req);

	auto const size = this->size;
	auto const capacity = this->capacity;

	if (size >= capacity) {
		if (req_size)
			return SinkRes_ErrOverflow;
		else
			return SinkRes_Ok;
	}

	usize const remain = capacity - size;

	usize write_size;
	SinkRes res;

	if (req_size > remain) {
		if (!(req & FLAG(SinkFlag_Partial)))
			return SinkRes_ErrOverflow;

		write_size = remain;
		res = SinkRes_Partial;
	} else {
		write_size = req_size;
		res = SinkRes_Ok;
	}

	memcpy(this->buffer + size, src, write_size);

	// we can safely cast to u32 because write size is <= remain
	this->size = size + (u32)write_size;
	if (aux) *(usize*)aux = write_size;

	return res;
}

ISink_GENERATE_KNOWN(BufferSink, BufferSink*)
