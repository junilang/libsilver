#define IPrintable_GENERATE_METHODS(N, E) \
	extern OutStreamRes IPrintable_##N(Ptr this, PrintFmt fmt, IARG(OutStream, os)) { \
		return N##_print((E)(usize)this, fmt, IWRAP(OutStream, os)); \
	}

#if Printable_PTRTAG
	#define IPrintable_GENERATE_UPCAST(N, E) \
		Printable N##_repr(E this) { \
			return (Printable){ptrtag((Ptr)(usize)this, IPrintable_##N##_ID)}; \
		}

	#define IPrintable_REGISTER(N) INTERFACE_REGISTER(IPrintable, N)
	#define IPrintable_REGISTER_KNOWN(N) INTERFACE_REGISTER_KNOWN(IPrintable, N)

#else
	#define IPrintable_GENERATE_UPCAST(N, E) \
		Printable N##_repr(E this) { \
			return (Printable){.this=(Ptr)(usize)this,.iface=&IPrintable_##N}; \
		}

	#define IPrintable_REGISTER(N)
	#define IPrintable_REGISTER_KNOWN(N)

#endif

#define IPrintable_GENERATE_STATIC(N, E) \
	FmtPrintable N##_fmtrepr(E this, PrintFmt fmt) { \
		return (FmtPrintable){.this=N##_repr(this),.fmt=fmt}; \
	} \
	StaticPrintable N##_srepr(E this) { \
		return (StaticPrintable){.this=(Ptr)(usize)this,.print=&IPrintable_##N}; \
	} \
	StaticFmtPrintable N##_sfmtrepr(E this, PrintFmt fmt) { \
		return (StaticFmtPrintable){\
			.this=(Ptr)(usize)this,.print=&IPrintable_##N,.fmt=fmt \
		}; \
	}

#define IPrintable_GENERATE_(N, E, REGISTER) \
	IPrintable_GENERATE_METHODS(N, E) \
	REGISTER(N) \
	IPrintable_GENERATE_UPCAST(N, E) \
	IPrintable_GENERATE_STATIC(N, E)

#define IPrintable_GENERATE_KNOWN(N, E) \
	IPrintable_GENERATE_(N, E, IPrintable_REGISTER_KNOWN)

#define IPrintable_GENERATE(N, E) \
	IPrintable_GENERATE_(N, E, IPrintable_REGISTER)
