#define IAlc_GENERATE_INTERFACE(N, E) \
	extern AlcPtr IAlc_##N(Ptr this, AlcReq *req, Ptr arg, Ptr mem) { \
		return N##_invoke((E)(usize)this, req, arg, mem); \
	}

#if Alc_PTRTAG
	#define IAlc_GENERATE_UPCAST(N, E) \
		Alc N##_upcast(E this) { \
			return (Alc){.value=ptrtag((Ptr)(usize)this, IAlc_##N##_ID)}; \
		}

	#define IAlc_REGISTER(N) INTERFACE_REGISTER(IAlc, N)
	#define IAlc_REGISTER_KNOWN(N) INTERFACE_REGISTER_KNOWN(IAlc, N)

#else
	#define IAlc_GENERATE_UPCAST(N, E) \
		Alc N##_upcast(E this) { \
			return (Alc){.this=(Ptr)(usize)this,.iface=&IAlc_##N}; \
		}

	#define IAlc_REGISTER(N)
	#define IAlc_REGISTER_KNOWN(N)

#endif

#define IAlc_GENERATE_(N, E, REGISTER) \
	IAlc_GENERATE_INTERFACE(N, E) \
	REGISTER(N) \
	IAlc_GENERATE_UPCAST(N, E)

#define IAlc_GENERATE(N, E) IAlc_GENERATE_(N, E, IAlc_REGISTER)
#define IAlc_GENERATE_KNOWN(N, E) IAlc_GENERATE_(N, E, IAlc_REGISTER_KNOWN)
