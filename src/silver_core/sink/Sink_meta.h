#define ISink_GENERATE_INTERFACE(N, E) \
	SinkRes ISink_##N(Ptr this, SinkReq req, Ptr aux, ConstPtr src) { \
		return N##_invoke((E)(usize)this, req, aux, src); \
	}

#if Sink_PTRTAG
	#define ISink_GENERATE_UPCAST(N, E) \
		Sink N##_asSink(E this) { \
			return (Sink){.value=ptrtag((Ptr)(usize)this, ISink_##N##_ID)}; \
		}

	#define ISink_REGISTER(N) IFACEDB_REGISTER(ISink, N)
	#define ISink_REGISTER_KNOWN(N) IFACEDB_SET(ISink, N)

#else
	#define ISink_GENERATE_UPCAST(N, E) \
		Sink N##_asSink(E this) { \
			return (Sink){.this=(Ptr)(usize)this,.iface=&ISink_##N}; \
		}

	#define ISink_REGISTER(N)
	#define ISink_REGISTER_KNOWN(N)

#endif

#define ISink_GENERATE_(N, E, REGISTER) \
	ISink_GENERATE_INTERFACE(N, E) \
	REGISTER(N) \
	ISink_GENERATE_UPCAST(N, E)

#define ISink_GENERATE(N, E) ISink_GENERATE_(N, E, ISink_REGISTER)
#define ISink_GENERATE_KNOWN(N, E) ISink_GENERATE_(N, E, ISink_REGISTER_KNOWN)
