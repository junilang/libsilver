#ifndef OutStream_PTRTAG
	#define OutStream_PTRTAG PTRTAG
#endif

#include "OutStreamRes.h"

typedef u32 OutStreamAttr;

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
		IOutStream_VoidOutStream_ID,
		IOutStream_BufferOutStream_ID,
		IOutStream_BufferedOutStream_ID,
		IOutStream_OsFileOutStream_ID,
		IOutStream_reserved
	};

	#ifndef IOutStream_SLOTS
		#define IOutStream_SLOTS 8
	#endif

	constexpr auto IOutStream_slots = IOutStream_SLOTS;

	IFACEDB(IOutStream, utag,
		IOutStream_slots,
		IOutStream_reserved
	)

	Ptr OutStream_this(OutStream this) { return ptrstrip(this.value); }
	const IOutStream *OutStream_iface(OutStream this) {
		return IFACEDB_GET(IOutStream, ptrread(this.value));
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
OutStreamRes OutStream_write(OutStream this, ConstPtr buf, usize buf_size) {
	return OutStream_iface(this)->write(OutStream_this(this), buf, buf_size);
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
extern OutStream os_stderr_stream();
extern OutStream os_stdout_stream();
