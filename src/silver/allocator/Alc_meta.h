#define IAlc_GENERATE_INTERFACE(N) \
	const IAlc IAlc_##N = { \
		.new = &IAlc_##N##_new, \
		.resize = &IAlc_##N##_resize, \
		.delete = &IAlc_##N##_delete, \
		.negotiate = &IAlc_##N##_negotiate, \
		.attr = &IAlc_##N##_attr, \
	};

#define IAlc_GENERATE_METHODS(N, E) \
	extern AlcAttr IAlc_##N##_attr(Ptr this) { \
		return N##_attr((E)(usize)this); \
	} \
	extern Ptr IAlc_##N##_new(Ptr this, AlcReq req, ConstPtr hint) { \
		return N##_new((E)(usize)this, req, hint); \
	} \
	extern Ptr IAlc_##N##_resize(Ptr this, Ptr mem, AlcReq req, ConstPtr hint) { \
		return N##_resize((E)(usize)this, mem, req, hint); \
	} \
	extern AlcRes IAlc_##N##_delete(Ptr this, Ptr mem, AlcReq req) { \
		return N##_delete((E)(usize)this, mem, req); \
	} \
	extern AlcNrs IAlc_##N##_negotiate(Ptr this, AlcNrq nrq, ConstPtr hint, usize *alts) { \
		return N##_negotiate((E)(usize)this, nrq, hint, alts); \
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
	IAlc_GENERATE_METHODS(N, E) \
	IAlc_GENERATE_INTERFACE(N) \
	REGISTER(N) \
	IAlc_GENERATE_UPCAST(N, E)

#define IAlc_GENERATE(N, E) IAlc_GENERATE_(N, E, IAlc_REGISTER)
#define IAlc_GENERATE_KNOWN(N, E) IAlc_GENERATE_(N, E, IAlc_REGISTER_KNOWN)
