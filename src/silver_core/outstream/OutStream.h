#ifndef OutStream_PTRTAG
	#define OutStream_PTRTAG PTRTAG
#endif

typedef u32 OutStreamAttr;

typedef enum : u8 {
	OutStreamRes_Ok,
	OutStreamRes_ErrInternal,
	OutStreamRes_ErrUnknown,
	OutStreamRes_ErrOverflow,
	OutStreamRes_ErrInvalid,
} OutStreamRes;

typedef struct {
	OutStreamRes (*write)(Ptr this, ConstPtr buffer, usize buffer_size);
	OutStreamRes (*flush)(Ptr this);
	OutStreamAttr (*attr)(Ptr this);
} IOutStream;

#if OutStream_PTRTAG
	typedef struct {
		union {
			Ptr value;
			usize raw_value;
		};
	} OutStream;

	enum {
		IOutStream_BufferOutStream_ID,
		IOutStream_BufferedOutStream_ID,
		IOutStream_OsFileOutStream_ID,
		IOutStream_KNOWN
	};

	INTERFACE_REGISTRY(IOutStream, utag, 4)

	Ptr OutStream_this(OutStream this) { return ptrstrip(this.value); }
	const IOutStream *OutStream_iface(OutStream this) {
		return &IOutStream__registry[ptrread(this.value)];
	}

	#define CONSTEXPR_OutStream_upcast(name, this) \
		{.raw_value=CONSTEXPR_ptrtag((this), IOutStream_##name##_ID)}

#else
	typedef struct {
		Ptr this;
		const IOutStream *iface;
	} OutStream;

	Ptr OutStream_this(OutStream this) { return this.this; }
	const IOutStream *OutStream_iface(OutStream this) { return this.iface; }

	#define CONSTEXPR_OutStream_upcast(name, this) \
		{.this=(Ptr)(usize)(this), .iface=&IOutStream_##name}

#endif

[[nodiscard]]
OutStreamRes OutStream_write(OutStream this, ConstPtr buffer, usize buffer_size) {
	return OutStream_iface(this)->write(OutStream_this(this), buffer, buffer_size);
}

[[nodiscard]]
OutStreamRes OutStream_flush(OutStream this) {
	return OutStream_iface(this)->flush(OutStream_this(this));
}

OutStreamAttr OutStream_attr(OutStream this) {
	return OutStream_iface(this)->attr(OutStream_this(this));
}

#include "OutStream_meta.h"

extern OutStream os_panic_stream();
