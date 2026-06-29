#ifndef Printable_PTRTAG
	#define Printable_PTRTAG PTRTAG
#endif

typedef struct {
	u32 value;
} PrintFmt;

constexpr PrintFmt PrintFmt_Null = { .value = 0 };

typedef OutStreamRes (*IPrintable)(Ptr this, PrintFmt fmt, IARG(OutStream, os));

#if Printable_PTRTAG
	typedef struct {
		Ptr value;
	} Printable;

	enum {
		IPrintable_KNOWN
	};

	INTERFACE_REGISTRY(IPrintable, utag, 64)

	Ptr Printable_this(Printable this) { return ptrstrip(this.value); }
	IPrintable Printable_iface(Printable this) {
		return IPrintable__registry[ptrread(this.value)];
	}

#else
	typedef struct {
		Ptr this;
		IPrintable iface;
	} Printable;

	Ptr Printable_this(Printable this) { return this.this; }
	IPrintable Printable_iface(Printable this) { return this.iface; }

#endif

[[nodiscard]]
OutStreamRes Printable_print(Printable this, PrintFmt fmt, OutStream os) {
	return Printable_iface(this)(Printable_this(this), fmt, IPASS(OutStream, os));
}

typedef struct {
	Printable this;
	PrintFmt fmt;
} FmtPrintable;

[[nodiscard]]
OutStreamRes FmtPrintable_print(FmtPrintable this, PrintFmt fmt, OutStream os) {
	return Printable_print(this.this, this.fmt, os);
}

typedef struct {
	Ptr this;
	IPrintable print;
} StaticPrintable;

[[nodiscard]]
OutStreamRes StaticPrintable_print(StaticPrintable this, PrintFmt fmt, OutStream os) {
	return this.print(this.this, fmt, os);
}

typedef struct {
	Ptr this;
	IPrintable print;
	PrintFmt fmt;
} StaticFmtPrintable;

[[nodiscard]]
OutStreamRes StaticFmtPrintable_print(StaticFmtPrintable this, PrintFmt fmt, OutStream os) {
	return this.print(this.this, this.fmt, os);
}

#include "Printable_meta.h"
