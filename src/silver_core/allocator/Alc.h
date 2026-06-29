#include "AlcRes.h"
#include "AlcReq.h"

typedef Ptr (*IAlc)(Ptr this, AlcReq *req, Ptr arg, Ptr mem);

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

	IAlc Alc_iface(Alc this) {
		return IAlc__registry[ptrread(this.value)];
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
		IAlc iface;
	} Alc;

	IAlc Alc_iface(Alc this) {
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
Ptr Alc_invoke(Alc this, AlcReq *req, Ptr arg, Ptr mem) {
	return Alc_iface(this)(Alc_this(this), req, arg, mem);
}

AlcAttr Alc_attr(Alc this) {
	// do not check for errors, all allocator implementations
	// 	must be able to report their attributes
	return (AlcAttr)(usize)Alc_invoke(this, &(AlcReq) {
		.intent = AlcIntent_Attr
	} ,nullptr, nullptr);
}

constexpr ualign Alc_default_align = alignof(Ptr);

[[nodiscard, gnu::malloc]]
Ptr Alc_new(Alc this, usize size) {
	return Alc_invoke(this, &(AlcReq) {
		.intent = AlcIntent_New,
		.size = size,
		.align = Alc_default_align
	}, nullptr, nullptr);
}

[[nodiscard]]
AlcRes Alc_delete(Alc this, Ptr mem) {
	return AlcRes_get(
		Alc_invoke(this, &(AlcReq) {
			.intent = AlcIntent_Delete,
		}, nullptr, mem)
	);
}

[[nodiscard, gnu::malloc]]
Ptr Alc_resize(Alc this, Ptr mem, usize size) {
	return Alc_invoke(this, &(AlcReq) {
		.intent = AlcIntent_Resize,
		.size = size,
		.align = Alc_default_align,
	}, nullptr, mem);
}

#include "Alc_meta.h"
