#define IAlc_GENERATE_INTERFACE(N) \
	const IAlc IAlc_##N = { \
		.attr = &IAlc_##N##_attr, \
		.new = &IAlc_##N##_new, \
		.resize = &IAlc_##N##_resize, \
		.delete = &IAlc_##N##_delete, \
		.query = &IAlc_##N##_query, \
		.resolve = &IAlc_##N##_resolve, \
		.lock = &IAlc_##N##_lock, \
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
	extern AlcRes IAlc_##N##_query(Ptr this, AlcReq req, ConstPtr hint, Ptr offers) { \
		return N##_query((E)(usize)this, req, hint, offers); \
	} \
	extern Ptr IAlc_##N##_resolve(Ptr this, AlcPromise *offers, AlcOffersSize offers_size, AlcOffersSize accept_index) { \
		return N##_resolve((E)(usize)this, offers, offers_size, accept_index); \
	} \
	extern AlcRes IAlc_##N##_lock(Ptr this, AlcLockIntent intent) { \
		return N##_lock((E)(usize)this, intent); \
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
