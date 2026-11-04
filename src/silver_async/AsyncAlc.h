typedef struct {
	_Atomic u32_lf lock;
	Alc provider;
} AsyncAlc;

AlcRes AsyncAlc_lock(AsyncAlc *this) {
	retry:;
	auto lock = atom_exchg(&this->lock, 1, atom_sync);

	if (!lock) return AlcRes_Ok;

	do {
		CPU_YIELD;
		lock = atom_get(&this->lock, atom_acq);
	} while (lock);

	goto retry;
}

AlcRes AsyncAlc_trylock(AsyncAlc *this) {
	auto const lock = atom_exchg(&this->lock, 1, atom_sync);
	if (!lock) return AlcRes_Ok;
	return AlcRes_Locked;
}

AlcRes AsyncAlc_unlock(AsyncAlc *this) {
	atom_set(&this->lock, 0, atom_rel);
	return AlcRes_Ok;
}

AlcPtr AsyncAlc_invoke(AsyncAlc *this, AlcReq *req, Ptr arg, Ptr mem) {
	constexpr AlcAttr attr =
		FLAGS(AlcAttr, ThreadSafe, IntentLock, IntentTryLock, IntentUnlock)
	;

	auto const intent = req->intent;
	switch (intent) {
		case AlcIntent_Lock: return AlcPtr_set(AsyncAlc_lock(this));
		case AlcIntent_TryLock: return AlcPtr_set(AsyncAlc_trylock(this));
		case AlcIntent_Unlock: return AlcPtr_set(AsyncAlc_unlock(this));
		case AlcIntent_Attr: return (AlcPtr)(usize)(Alc_attr(this->provider) | attr);
		default:;
	}

	bool const locked = req->flags & FLAG(AlcFlag_Locked);
	if (locked) {
		req->flags &= FLAG_CLEAR(AlcFlag_Locked);
	} else {
		if (req->flags & FLAG(AlcFlag_NoWait)) {
			auto const res = AsyncAlc_trylock(this);
			if (res) return AlcPtr_set(res);
		} else {
			AsyncAlc_lock(this);
		}
	}

	auto const res = Alc_invoke(this->provider, req, arg, mem);

	if (locked) {
		req->flags |= FLAG(AlcFlag_Locked);
	} else {
		AsyncAlc_unlock(this);
	}

	return res;
}

IAlc_GENERATE_KNOWN(AsyncAlc, AsyncAlc*)

void AsyncAlc_init(AsyncAlc *this, Alc provider) {
	this->provider = provider;
	this->lock = 0;
}
