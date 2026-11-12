#ifndef BufferedSink_SAFE
	#define BufferedSink_SAFE LIBSILVER_SAFE
#endif

typedef struct {
	Sink sink;
	ubyte *buffer;
	u32 size;
	u32 capacity;
} BufferedSink;

void BufferedSink_init(BufferedSink *this, Sink sink, ubyte *buffer, u32 capacity) {
	this->sink = sink;
	this->buffer = buffer;
	this->size = 0;
	this->capacity = capacity;
}

SinkRes BufferedSink_push_(BufferedSink *this, SinkReq req) {
	auto const sink = this->sink;
	auto const buffer = this->buffer;
	auto const size = this->size;

	if (!size) return SinkRes_Ok;


	SinkReq const propagate = req & FLAGS(SinkFlag, NoWait);
	auto const res = Sink_invoke(sink,
		FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, size) | propagate
	, nullptr, buffer);
	if (res) return res;

	this->size = 0;
	return SinkRes_Ok;
}

SinkRes BufferedSink_flush_(BufferedSink *this, SinkReq req) {
	auto const sink = this->sink;
	auto const buffer = this->buffer;
	auto const size = this->size;

	if (!size) {
		//return Sink_invoke()
	}

	auto const res = Sink_invoke(sink,
		FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, size) | req
	, nullptr, buffer);
	if (res) return res;

	this->size = 0;
	return SinkRes_Ok;
}

SinkRes BufferedSink_push(BufferedSink *this) {
	return BufferedSink_push_(this, 0);
}

SinkRes BufferedSink_flush(BufferedSink *this) {
	return BufferedSink_flush_(this, 0);
}

SinkRes BufferedSink_invoke(BufferedSink *this, SinkReq req, Ptr aux, ConstPtr src) {
	auto constexpr attr = FLAGS_CAST(SinkAttr,
		IntentWrite, IntentFlush, IntentPush
	);

	SinkReq constexpr propagate_push =
		FLAGS(SinkFlag, NoWait)
	;

	SinkReq constexpr propagate_handoff =
		FIELD_MASK(SinkSize) | FIELD_MASK(SinkIntent) |
		FLAGS(SinkFlag, Flush, NoWait, Partial)
	;

	switch (FIELD_GET_CAST(SinkIntent, req)) {
		default:
			return SinkRes_ErrUnsupported;

		case SinkIntent_Push:
			return BufferedSink_push_(this, req);

		case SinkIntent_Flush:
			return BufferedSink_flush_(this, req);

		case SinkIntent_Attr:
			*(SinkAttr*)aux = attr;
			return SinkRes_Ok;

		case SinkIntent_Write:
	}

	usize const req_size = FIELD_GET(SinkSize, req);

	auto const sink = this->sink;
	auto const buffer = this->buffer;
	auto const size = this->size;
	auto const capacity = this->capacity;

	#if BufferedSink_SAFE
		ASSERT(size < capacity);
	#endif

	if (req_size > capacity) {
		if (size) goto push_and_fwd;
		else goto fwd;
	}

	auto const remain = capacity - size;

	bool push = req & FLAGS(SinkFlag, Flush, Push);

	if (remain < req_size) {
		if (push) {
			push_and_fwd:;
			auto const res = Sink_invoke(sink,
				FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, size) | (req & propagate_push)
			, nullptr, buffer);

			if (res) {
				if (aux) *(usize*)aux = 0;
				return res;
			}

			this->size = 0;

			fwd:;
			return Sink_invoke(sink, req & propagate_handoff, aux, src);

		} else {
			memcpy(buffer + size, src, remain);
			auto const res = Sink_invoke(sink,
				FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, capacity) | (req & propagate_push)
			, nullptr, buffer);
			if (res) {
				if (aux) *(usize*)aux = 0;
				return res;
			}

			memcpy(buffer, (const ubyte*)src + remain, req_size - remain);
			if (aux) *(usize*)aux = req_size;

			return SinkRes_Ok;
		}
	}

	push = push || remain == req_size;

	memcpy(this->buffer + size, src, req_size);
	if (aux) *(usize*)aux = req_size;

	auto const new_size = size + (u32)req_size;

	if (push) {
		auto const res = Sink_invoke(this->sink,
			FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, new_size) |
			(req & FLAG(SinkFlag_Flush))
		, nullptr, buffer);
		if (res) return res;

		this->size = 0;
		return SinkRes_Ok;
	}

	this->size = size + (u32)req_size;
	return SinkRes_Ok;
}

ISink_GENERATE_KNOWN(BufferedSink, BufferedSink*)
