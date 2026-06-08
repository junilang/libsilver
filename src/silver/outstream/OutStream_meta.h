#define IOutStream_GENERATE_INTERFACE(N) \
	const IOutStream IOutStream_##N = { \
		.write = &IOutStream_##N##_write, \
		.flush = &IOutStream_##N##_flush \
	};

#define IOutStream_GENERATE_METHODS(N, E) \
	extern void IOutStream_##N##_write(Ptr this, ConstPtr buffer, usize buffer_size) { \
		N##_write((E)(usize)this, buffer, buffer_size); \
	} \
	extern void IOutStream_##N##_flush(Ptr this) { \
		N##_flush((E)(usize)this); \
	}

#if OutStream_PTRTAG
	#define IOutStream_GENERATE_UPCAST(N, E) \
		OutStream N##_upcast(E this) { \
			return (OutStream){ptrtag((Ptr)(usize)this, IOutStream_##N##_ID)}; \
		}

	#define IOutStream_REGISTER(N) INTERFACE_REGISTER(IOutStream, N)
	#define IOutStream_REGISTER_KNOWN(N) INTERFACE_REGISTER_KNOWN(IOutStream, N)

#else
	#define IOutStream_GENERATE_UPCAST(N, E) \
		OutStream N##_upcast(E this) { \
			return (OutStream){.this=(Ptr)(usize)this,.iface=&IOutStream_##N}; \
		}

	#define IOutStream_REGISTER(N)
	#define IOutStream_REGISTER_KNOWN(N)

#endif

#define IOutStream_GENERATE_(N, E, REGISTER) \
	IOutStream_GENERATE_METHODS(N, E) \
	IOutStream_GENERATE_INTERFACE(N) \
	IOutStream_GENERATE_UPCAST(N, E) \
	REGISTER(N)

#define IOutStream_GENERATE(N, E) \
	IOutStream_GENERATE_(N, E, IOutStream_REGISTER)

#define IOutStream_GENERATE_KNOWN(N, E) \
	IOutStream_GENERATE_(N, E, IOutStream_REGISTER_KNOWN)
