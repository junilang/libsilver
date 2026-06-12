#include "AlcReq.h"
#include "AlcNrq.h"

typedef u32 AlcAttr; enum {
	AlcAttr_BIT_ThreadSafe,
	AlcAttr_BIT_NoResize
};

typedef struct {
	Ptr (*new)(Ptr this, AlcReq req, ConstPtr hint);
	Ptr (*resize)(Ptr this, Ptr mem, AlcReq req, ConstPtr hint);
	AlcRes (*delete)(Ptr this, Ptr mem, AlcReq req);
	AlcNrs (*negotiate)(Ptr this, AlcNrq nrq, ConstPtr hint, usize *alts);
	AlcAttr (*attr)(Ptr this);
} IAlc;

#ifndef Alc_PTRTAG
	#define Alc_PTRTAG PTRTAG
#endif

#if Alc_PTRTAG
	typedef union {
		Ptr value;
		usize raw_value;
	} Alc;

	enum {
		IAlc_StaticAlc_ID,
		IAlc_OsAlc_ID,
		IAlc_ArenaAlc_ID,
		IAlc_KNOWN
	};

	INTERFACE_REGISTRY(IAlc, utag, 8)

	const IAlc *Alc_iface(Alc this) {
		return &IAlc__registry[ptrread(this.value)];
	}

	Ptr Alc_this(Alc this) {
		return ptrstrip(this.value);
	}

	bool Alc_equal(Alc this, Alc other) {
		return this.value == other.value;
	}

	#define CONSTEXPR_Alc_upcast(name, this) \
		{.raw_value=CONSTEXPR_ptrtag((this), IAlc_##name##_ID)}

#else
	typedef struct {
		Ptr this;
		const IAlc *iface;
	} Alc;

	const IAlc *Alc_iface(Alc this) {
		return this.iface;
	}

	Ptr Alc_this(Alc this) {
		return this.this;
	}

	bool Alc_equal(Alc this, Alc other) {
		return this.value == other.value && this.iface == other.iface;
	}

	#define CONSTEXPR_Alc_upcast(name, this) \
		{.this=(Ptr)(usize)(this), .iface=&IAlc_##name)}

#endif

[[nodiscard, gnu::malloc]]
Ptr Alc_new(Alc this, AlcReq req, ConstPtr hint) {
	return Alc_iface(this)->new(Alc_this(this), req, hint);
}

[[nodiscard]]
Ptr Alc_resize(Alc this, Ptr mem, AlcReq req, ConstPtr hint) {
	return Alc_iface(this)->resize(Alc_this(this), mem, req, hint);
}

[[nodiscard]]
AlcRes Alc_delete(Alc this, Ptr mem, AlcReq req) {
	return Alc_iface(this)->delete(Alc_this(this), mem, req);
}

[[nodiscard]]
AlcNrs Alc_negotiate(Alc this, AlcNrq req, ConstPtr hint, usize *alts) {
	return Alc_iface(this)->negotiate(Alc_this(this), req, hint, alts);
}

AlcAttr Alc_attr(Alc this) {
	return Alc_iface(this)->attr(Alc_this(this));
}

#include "Alc_meta.h"
