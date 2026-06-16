#include "AlcReq.h"
#include "AlcRes.h"

typedef u32 AlcAttr; enum {
	AlcAttr_BIT_ThreadSafe,
	AlcAttr_BIT_FeatureResize,
	AlcAttr_BIT_FeatureRequest,
	AlcAttr_BIT_FeaturePromise,
	AlcAttr_BIT_FeatureLock,
	AlcAttr_BIT_FeatureZero,
	AlcAttr_BIT_FeatureEmbedSize,
	AlcAttr_BIT_FeatureRelativeLocal,
	AlcAttr_BIT_FeatureRelativeS16,
	AlcAttr_BIT_FeatureRelativeU16,
	AlcAttr_BIT_FeatureRelativeS32,
	AlcAttr_BIT_FeatureRelativeU32
};

typedef enum : u8 {
	AlcLockIntent_Lock,
	AlcLockIntent_Unlock
} AlcLockIntent;

typedef struct {
	AlcAttr (*attr)(Ptr this);

	Ptr (*new)(Ptr this, AlcReq req, ConstPtr hint);
	Ptr (*resize)(Ptr this, Ptr mem, AlcReq req, ConstPtr hint);
	AlcRes (*delete)(Ptr this, Ptr mem, AlcReq req);

	AlcRes (*query)(Ptr this, AlcReq req, ConstPtr hint, Ptr offers);
	Ptr (*resolve)(Ptr this, AlcPromise *offers, u8 offers_size, u8 accept_index);

	AlcRes (*lock)(Ptr this, AlcLockIntent intent);
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
AlcRes Alc_query(Alc this, AlcReq req, ConstPtr hint, Ptr offers) {
	return Alc_iface(this)->query(Alc_this(this), req, hint, offers);
}

[[nodiscard, gnu::malloc]]
Ptr Alc_resolve(Alc this, AlcPromise *offers, AlcOffersSize offers_size, AlcOffersSize accept_index) {
	return Alc_iface(this)->resolve(Alc_this(this), offers, offers_size, accept_index);
}

AlcRes Alc_lock(Alc this) {
	return Alc_iface(this)->lock(Alc_this(this), AlcLockIntent_Lock);
}

AlcRes Alc_unlock(Alc this) {
	return Alc_iface(this)->lock(Alc_this(this), AlcLockIntent_Lock);
}

AlcAttr Alc_attr(Alc this) {
	return Alc_iface(this)->attr(Alc_this(this));
}

#include "Alc_meta.h"
