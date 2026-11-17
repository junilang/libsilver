#ifndef Fmt_PTRTAG
	#define Fmt_PTRTAG PTRTAG
#endif

typedef usize FmtSpec;
FmtSpec constexpr FmtSpec_null = 0;

typedef SinkRes (*IFmt)(Ptr this, FmtSpec spec, IARG(Sink, sink));

typedef struct {
	Ptr this;
	IFmt iface;
} Fmt;

Ptr Fmt_this(Fmt this) {
	return this.this;
}

IFmt Fmt_iface(Fmt this) {
	return this.iface;
}

[[nodiscard]]
SinkRes Fmt_invoke(Fmt this, FmtSpec spec, Sink sink) {
	return Fmt_iface(this)(Fmt_this(this), spec, IPASS(Sink, sink));
}

#define IFmt_GENERATE(N, E) \
	SinkRes IFmt_##N(Ptr this, FmtSpec spec, IARG(Sink, sink)) { \
		return N##_fmt((E)(usize)this, spec, IWRAP(Sink, sink)); \
	} \
	Fmt N##_asFmt(E this) { \
		return (Fmt){.this=(Ptr)(usize)this,.iface=&IFmt_##N}; \
	}
