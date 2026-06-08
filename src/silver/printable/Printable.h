#ifndef Printable_PTRTAG
	#define Printable_PTRTAG PTRTAG
#endif

typedef struct {
	u32 value;
} PrintFmt;

constexpr PrintFmt PrintFmt_NULL = { .value = 0 };

typedef void (*PrintFn)(Ptr this, PrintFmt fmt, IARG(OutStream, os));

typedef struct {
	PrintFn print;
} IPrintable;

#if Printable_PTRTAG
	typedef struct {
		Ptr value;
	} Printable;

	enum {
		IPrintable_UintPrintable_ID,
		IPrintable_IntPrintable_ID,
		IPrintable_KNOWN
	};

	INTERFACE_REGISTRY(IPrintable, utag, 64)

	Ptr Printable_this(Printable this) { return ptrstrip(this.value); }
	const IPrintable *Printable_iface(Printable this) {
		return &IPrintable__registry[ptrread(this.value)];
	}

#else
	typedef struct {
		Ptr this;
		const IPrintable *iface;
	} Printable;

	Ptr Printable_this(Printable this) { return this.this; }
	const IPrintable *Printable_iface(Printable this) { return this.iface; }

#endif

void Printable_print(Printable this, PrintFmt fmt, OutStream os) {
	Printable_iface(this)->print(Printable_this(this), fmt, IPASS(OutStream, os));
}

typedef struct {
	Printable this;
	PrintFmt fmt;
} FmtPrintable;

void FmtPrintable_print(FmtPrintable this, PrintFmt fmt, OutStream os) {
	Printable_print(this.this, this.fmt, os);
}

typedef struct {
	Ptr this;
	PrintFn print;
} StaticPrintable;

void StaticPrintable_print(StaticPrintable this, PrintFmt fmt, OutStream os) {
	this.print(this.this, fmt, os);
}

typedef struct {
	Ptr this;
	PrintFn print;
	PrintFmt fmt;
} StaticFmtPrintable;

void StaticFmtPrintable_print(StaticFmtPrintable this, PrintFmt fmt, OutStream os) {
	this.print(this.this, this.fmt, os);
}

#include "Printable_meta.h"
