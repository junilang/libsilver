#define IAsyncRT_GENERATE_INTERFACE(N) \
	const IAsyncRT IAsyncRT_##N = { \
		.submit = &IAsyncRT_##N##_submit, \
		.resolve = &IAsyncRT_##N##_resolve, \
	};

#define IAsyncRT_GENERATE_METHODS(N) \
	extern void IAsyncRT_##N##_submit(Ptr this, const AsyncTask *tasks, usize tasks_size) { \
		N##_submit(this, tasks, tasks_size); \
	} \
	extern void IAsyncRT_##N##_resolve(Ptr this, const AsyncFuture *futures, usize futures_size) { \
		N##_resolve(this, futures, futures_size); \
	}

#if AsyncRT_PTRTAG
	#define IAsyncRT_GENERATE_UPCAST(N) \
		AsyncRT N##_upcast(Ptr this) { \
			return (AsyncRT){ptrtag(this, IAsyncRT_##N##_ID)}; \
		}

	#define IAsyncRT_REGISTER(N) INTERFACE_REGISTER(IAsyncRT, N)
	#define IAsyncRT_REGISTER_KNOWN(N) INTERFACE_REGISTER_KNOWN(IAsyncRT, N)

#else
	#define IAsyncRT_GENERATE_UPCAST(N) \
		AsyncRT N##_upcast(Ptr this) {
			return (AsyncRT){.this=-this,.iface=&IAsyncRT_##N};
		}

	#define IAsyncRt_REGISTER(N)
	#define IAsyncRt_REGISTER_KNOWN(N)

#endif

#define IAsyncRT_GENERATE_(N, REGISTER) \
	IAsyncRT_GENERATE_METHODS(N) \
	IAsyncRT_GENERATE_INTERFACE(N) \
	REGISTER(N) \
	IAsyncRT_GENERATE_UPCAST(N)

#define IAsyncRT_GENERATE(N) IAsyncRT_GENERATE_(N, IAsyncRT_REGISTER)
#define IAsyncRT_GENERATE_KNOWN(N) IAsyncRT_GENERATE_(N, IAsyncRT_REGISTER_KNOWN)

#define IAsyncRT_DECLARE(N) \
	AsyncRT N##_upcast(Ptr this);
