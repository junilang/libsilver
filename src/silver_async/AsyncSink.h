typedef struct {
	Sink sink;
	_Atomic u32_lf lock;
} AsyncSink;

SinkRes AsyncSink_lock(AsyncSink *this, SinkReq req) {
	if (req & FLAG(SinkFlag_NoWait)) {
		if (atom_exchg(&this->lock, 1, atom_sync) == 0)
			return SinkRes_Ok;
		else
			return SinkRes_Locked;
	}

	retry:;
	auto lock = atom_exchg(&this->lock, 1, atom_sync);
	if (lock == 0) return SinkRes_Ok;

	do {
		CPU_YIELD;
		lock = atom_get(&this->lock, atom_acq);
	} while (lock);

	goto retry;
}

SinkRes AsyncSink_unlock(AsyncSink *this, SinkReq req) {
	if (!(req & FLAG(SinkFlag_Locked)))
		return SinkRes_ErrInvalidReq;

	atom_set(&this->lock, 0, atom_rel);
	return SinkRes_Ok;
}

SinkRes AsyncSink_invoke(AsyncSink *this, SinkReq req, Ptr aux, ConstPtr src) {
	auto constexpr attr = FLAGS_CAST(SinkAttr,
		IntentLock, IntentUnlock, Concurrent
	);

	switch (FIELD_GET_CAST(SinkIntent, req)) {
		case SinkIntent_Lock:
			return AsyncSink_lock(this, req);
		case SinkIntent_Unlock:
			return AsyncSink_unlock(this, req);

		case SinkIntent_Attr:
			if (!aux) return SinkRes_ErrInvalidAux;
			*(SinkAttr*)aux = Sink_attr(this->sink) | attr;
			return SinkRes_Ok;

		default:
	}

	bool const locked = req & FLAG(SinkFlag_Locked);
	auto const mod_req = req & FLAG_CLEAR(SinkFlag_Locked);

	if (!locked) {
		auto const res = AsyncSink_lock(this, req);
		if (res) return res;
	}

	auto const res = Sink_invoke(this->sink, mod_req, aux, src);

	if (!locked) {
		auto const res = AsyncSink_unlock(this, FLAG(SinkFlag_Locked));
		if (res) return res;
	}

	return res;
}

ISink_GENERATE_KNOWN(AsyncSink, AsyncSink*)

void AsyncSink_init(AsyncSink *this, Sink sink) {
	this->sink = sink;
	this->lock = 0;
}
