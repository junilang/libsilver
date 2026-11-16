typedef struct {
	Alc provider;
	_Atomic u32_lf lock;
} AsyncAlc;

AlcRes AsyncAlc_lock_(AsyncAlc *this, AlcReq req) {
	if (req & FLAG(AlcFlag_NoWait)) {
		if (atom_exchg(&this->lock, 1, atom_sync) == 0)
			return AlcRes_Ok;
		else
			return AlcRes_Locked;
	}

	retry:;
	auto lock = atom_exchg(&this->lock, 1, atom_sync);
	if (lock == 0) return AlcRes_Ok;

	do {
		CPU_YIELD;
		lock = atom_get(&this->lock, atom_acq);
	} while (lock);

	goto retry;
}

AlcRes AsyncAlc_unlock_(AsyncAlc *this, AlcReq req) {
	if (!(req & FLAG(AlcFlag_Locked)))
		return AlcRes_ErrInvalidReq;

	if (atom_exchg(&this->lock, 0, atom_sync) == 0)
		return AlcRes_NotLocked;
	else
		return AlcRes_Ok;
}

AlcPtr AsyncAlc_invoke(AsyncAlc *this, AlcReq req, Ptr aux, Ptr mem) {
	AlcAttr constexpr attr = FLAGS(AlcAttr,
		IntentLock, IntentUnlock, Concurrent
	);

	switch (FIELD_GET_CAST(AlcIntent, req)) {
		case AlcIntent_Lock:
			return AlcPtr_set(AsyncAlc_lock_(this, req));
		case AlcIntent_Unlock:
			return AlcPtr_set(AsyncAlc_unlock_(this, req));

		case AlcIntent_Attr:
			if (!aux) return AlcPtr_set(AlcRes_ErrInvalidAux);
			*(AlcAttr*)aux = Alc_attr(this->provider) | attr;
			return AlcPtr_Ok;

		default:
	}

	bool const locked = req & FLAG(AlcFlag_Locked);
	auto const mod_req = req & FLAG_CLEAR(AlcFlag_Locked);

	if (!locked) {
		auto const res = AsyncAlc_lock_(this, req);
		if (res) return AlcPtr_set(res);
	}

	auto const res = Alc_invoke(this->provider, mod_req, aux, mem);

	if (!locked) {
		auto const res = AsyncAlc_unlock_(this, FLAG(AlcFlag_Locked));
		if (res) return AlcPtr_set(res);
	}

	return res;
}

IAlc_GENERATE_KNOWN(AsyncAlc, AsyncAlc*)

void AsyncAlc_init(AsyncAlc *this, Alc provider) {
	this->provider = provider;
	this->lock = 0;
}
