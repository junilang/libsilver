#ifndef AlignedAllocator_PTRTAG
	#define AlignedAllocator_PTRTAG PTRTAG
#endif

#ifndef AlignedAllocator_SAFE
	#define AlignedAllocator_SAFE BUILD_SAFE
#endif

enum {
	AlignedAllocatorAttr_BIT_THREADSAFE
};

typedef u8 AlignedAllocatorAttr;

typedef struct {
	AlignedAllocatorAttr (*attr)(Ptr this);
	Ptr (*new)(Ptr this, ualign align, usize size);
	Ptr (*resize)(Ptr this, ualign align, Ptr buf, usize size);
	void (*delete)(Ptr this, ualign align, Ptr buf);
} IAlignedAllocator;

#if AlignedAllocator_PTRTAG
	typedef struct {
		Ptr value;
	} AlignedAllocator;

	enum {
		IAlignedAllocator_AlignedMalloc_ID,
		IAlignedAllocator_KNOWN
	};

	INTERFACE_REGISTRY(IAlignedAllocator, utag, 8)

	Ptr AlignedAllocator_this(AlignedAllocator this) { return ptrstrip(this.value); }
	const IAlignedAllocator *AlignedAllocator_iface(AlignedAllocator this) {
		return &IAlignedAllocator__registry[ptrread(this.value)];
	}

	bool AlignedAllocator_equal(AlignedAllocator this, AlignedAllocator other) {
		return this.value == other.value;
	}

#else
	typedef struct {
		Ptr this;
		const IAlignedAllocator *iface;
	} AlignedAllocator;

	Ptr AlignedAllocator_this(AlignedAllocator this) { return this.this; }
	const IAlignedAllocator *AlignedAllocator_iface(AlignedAllocator this) { return this.iface; }

	bool AlignedAllocator_equal(AlignedAllocator this, AlignedAllocator other) {
		return this.this == other.this && this.iface == other.iface;
	}

#endif

Ptr AlignedAllocator_new(AlignedAllocator this, ualign align, usize size) {
	return AlignedAllocator_iface(this)->new(AlignedAllocator_this(this), align, size);
}

Ptr AlignedAllocator_resize(AlignedAllocator this, ualign align, Ptr buf, usize size) {
	return AlignedAllocator_iface(this)->resize(AlignedAllocator_this(this), align, buf, size);
}

void AlignedAllocator_delete(AlignedAllocator this, ualign align, Ptr buf) {
	AlignedAllocator_iface(this)->delete(AlignedAllocator_this(this), align, buf);
}

AlignedAllocatorAttr AlignedAllocator_attr(AlignedAllocator this) {
	return AlignedAllocator_iface(this)->attr(AlignedAllocator_this(this));
}

#include "AlignedAllocator_meta.h"
