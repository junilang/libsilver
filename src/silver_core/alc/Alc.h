#ifndef Alc_PTRTAG
	#define Alc_PTRTAG PTRTAG
#endif

#ifndef Alc_SAFE
	#define Alc_SAFE LIBSILVER_SAFE
#endif

#include "AlcReq.h"
#include "AlcRes.h"
#include "AlcAttr.h"

typedef AlcPtr (*IAlc)(Ptr this, AlcReq req, Ptr aux, Ptr mem);

#if Alc_PTRTAG
	typedef struct {
		union {
			Ptr value;
			usize raw_value;
		};
	} Alc;

	enum {
		IAlc_VoidAlc_ID,
		IAlc_BumpAlc_ID,
		IAlc_AsyncAlc_ID,
		IAlc_OsAlc_ID,
		IAlc_reserved
	};

	#ifndef IAlc_SLOTS
		#define IAlc_SLOTS 8
	#endif

	constexpr auto IAlc_slots = IAlc_SLOTS;

	IFACEDB(IAlc, utag, IAlc_slots, IAlc_reserved)

	IAlc Alc_iface(Alc this) {
		return *IFACEDB_GET(IAlc, ptrread(this.value));
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

[[nodiscard]]
AlcPtr Alc_invoke(Alc this, AlcReq req, Ptr aux, Ptr mem) {
	return Alc_iface(this)(Alc_this(this), req, aux, mem);
}

AlcAttr Alc_attr(Alc this) {
	AlcAttr result;
	[[maybe_unused]] auto const ptr =
		Alc_invoke(this, FIELD(AlcIntent, Attr), &result, nullptr)
	;
	#if Alc_SAFE
		AlcPtr_ASSERT(ptr);
	#endif
	return result;
}

AlcRes Alc_delete(Alc this, Ptr mem) {
	return AlcPtr_get(Alc_invoke(this, AlcReq_delete, nullptr, mem));
}

#include "Alc_meta.h"
