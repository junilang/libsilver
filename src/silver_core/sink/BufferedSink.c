#ifndef BufferedSink_SAFE
	#define BufferedSink_SAFE LIBSILVER_SAFE
#endif

typedef struct {
	Sink sink;
	ubyte *buffer;
	usize size;
	usize capacity;
	bool locked;
} BufferedSink;

void BufferedSink_init(BufferedSink *this, Sink sink, ubyte *buffer, usize capacity) {
	this->sink = sink;
	this->buffer = buffer;
	this->size = 0;
	this->capacity = capacity;
	this->locked = false;
}

SinkRes BufferedSink_lock(BufferedSink *this) {
	if (this->locked)
		return SinkRes_Locked;

	auto const res = Sink_invoke(this->sink,
		FIELD(SinkIntent, Lock)
	, nullptr, nullptr);
	if (res) return res;

	this->locked = true;
	return SinkRes_Ok;
}

SinkRes BufferedSink_unlock(BufferedSink *this) {
	if (!this->locked)
		return SinkRes_NotLocked;

	auto const res = Sink_invoke(this->sink,
		FIELD(SinkIntent, Unlock) | FLAG(SinkFlag_Locked)
	, nullptr, nullptr);
	if (res) return res;

	this->locked = false;
	return SinkRes_Ok;
}

SinkRes BufferedSink_lock_(BufferedSink *this, SinkReq req) {
	if (this->locked) return SinkRes_Ok;

	auto constexpr propagate = FLAGS(SinkFlag, Locked, NoWait);
	return Sink_invoke(this->sink,
		FIELD(SinkIntent, Lock) | (req & propagate)
	, nullptr, nullptr);
}

SinkRes BufferedSink_unlock_(BufferedSink *this, SinkReq req) {
	if (this->locked) return SinkRes_Ok;

	auto constexpr propagate = FLAGS(SinkFlag, Locked, NoWait);
	return Sink_invoke(this->sink,
		FIELD(SinkIntent, Unlock) | (req & propagate)
	, nullptr, nullptr);
}

SinkRes BufferedSink_push_(BufferedSink *this, SinkReq req) {
	auto const size = this->size;

	if (!size) return SinkRes_Ok;

	auto constexpr propagate = FLAGS(SinkFlag, Locked, NoWait);
	auto const res = Sink_invoke(this->sink,
		FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, size) |
		(req & propagate) | (this->locked ? FLAG(SinkFlag_Locked) : 0)
	, nullptr, this->buffer);
	if (res) return res;

	this->size = 0;
	return SinkRes_Ok;
}

SinkRes BufferedSink_flush_(BufferedSink *this, SinkReq req) {
	auto const size = this->size;
	auto constexpr propagate = FLAGS(SinkFlag, Locked, NoWait);

	if (!size) {
		auto const res = Sink_invoke(this->sink,
			FIELD(SinkIntent, Flush) | (req & propagate) |
			(this->locked ? FLAG(SinkFlag_Locked) : 0)
		, nullptr, nullptr);
		if (res) return res;
		return SinkRes_Ok;
	}

	auto const res = Sink_invoke(this->sink,
		FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, size) |
		FLAG(SinkFlag_Flush) | (req & propagate) |
		(this->locked ? FLAG(SinkFlag_Locked) : 0)
	, nullptr, this->buffer);
	if (res) return res;

	this->size = 0;
	return SinkRes_Ok;
}

SinkRes BufferedSink_invoke(BufferedSink *this, SinkReq req, Ptr aux, ConstPtr src) {
	auto constexpr attr = FLAGS_CAST(SinkAttr,
		IntentWrite, IntentFlush, IntentPush
	);

	SinkReq constexpr propagate_push =
		FLAGS(SinkFlag, Locked, NoWait)
	;

	SinkReq constexpr propagate_handoff =
		FIELD_MASK(SinkSize) | FIELD_MASK(SinkIntent) |
		FLAGS(SinkFlag, Flush, Locked, NoWait, Partial)
	;

	switch (FIELD_GET_CAST(SinkIntent, req)) {
		default:
			return SinkRes_ErrUnsupported;

		case SinkIntent_Push:
			return BufferedSink_push_(this, req);

		case SinkIntent_Flush:
			return BufferedSink_flush_(this, req);

		case SinkIntent_Lock:
			return BufferedSink_lock_(this, req);

		case SinkIntent_Unlock:
			return BufferedSink_unlock_(this, req);

		case SinkIntent_Attr:
			*(SinkAttr*)aux = Sink_attr(this->sink) | attr;
			return SinkRes_Ok;

		case SinkIntent_Write:
	}

	usize const req_size = FIELD_GET(SinkSize, req);
	SinkReq const global_lock = this->locked ? FLAG(SinkFlag_Locked) : 0;


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
				FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, size) |
				(req & propagate_push) | global_lock
			, nullptr, buffer);

			if (res) {
				if (aux) *(usize*)aux = 0;
				return res;
			}

			this->size = 0;

			fwd:;
			return Sink_invoke(sink, (req & propagate_handoff) | global_lock, aux, src);

		} else {
			memcpy(buffer + size, src, remain);
			auto const res = Sink_invoke(sink,
				FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, capacity) |
				(req & propagate_push) | global_lock
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

	auto const new_size = size + req_size;

	if (push) {
		auto const res = Sink_invoke(this->sink,
			FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, new_size) |
			(req & FLAG(SinkFlag_Flush))
		, nullptr, buffer);
		if (res) return res;

		this->size = 0;
		return SinkRes_Ok;
	}

	this->size = size + req_size;
	return SinkRes_Ok;
}

ISink_GENERATE_KNOWN(BufferedSink, BufferedSink*)
