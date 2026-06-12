#define IPrintable_GENERATE_INTERFACE(N) \
	const IPrintable IPrintable_##N = { \
		.print = &IPrintable_##N##_print \
	};

#define IPrintable_GENERATE_METHODS(N, E) \
	extern void IPrintable_##N##_print(Ptr this, PrintFmt fmt,  IARG(OutStream, os)) { \
		N##_print((E)(usize)this, fmt, IWRAP(OutStream, os)); \
	}

#if Printable_PTRTAG
	#define IPrintable_GENERATE_UPCAST(N, E) \
		Printable N##_repr(E this) { \
			return (Printable){ptrtag((Ptr)(usize)this, IPrintable_##N##_ID)}; \
		}

	#define IPrintable_REGISTER(N) INTERFACE_REGISTER(IPrintable, N)
	#define IPrintable_REGISTER_KNOWN(N) INTERFACE_REGISTER(IPrintable, N)

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
		return (StaticPrintable){.this=(Ptr)(usize)this,.print=&IPrintable_##N##_print}; \
	} \
	StaticFmtPrintable N##_sfmtrepr(E this, PrintFmt fmt) { \
		return (StaticFmtPrintable){\
			.this=(Ptr)(usize)this,.print=&IPrintable_##N##_print,.fmt=fmt \
		}; \
	}

#define IPrintable_GENERATE_(N, E, REGISTER) \
	IPrintable_GENERATE_METHODS(N, E) \
	IPrintable_GENERATE_INTERFACE(N) \
	REGISTER(N) \
	IPrintable_GENERATE_UPCAST(N, E) \
	IPrintable_GENERATE_STATIC(N, E)

#define IPrintable_GENERATE_KNOWN(N, E) \
	IPrintable_GENERATE_(N, E, IPrintable_REGISTER_KNOWN)

#define IPrintable_GENERATE(N, E) \
	IPrintable_GENERATE_(N, E, IPrintable_REGISTER)
