#define IAllocator_GENERATE_INTERFACE(N) \
	const IAllocator IAllocator_##N = { \
		.attr = &IAllocator_##N##_attr, \
		.new = &IAllocator_##N##_new, \
		.resize = &IAllocator_##N##_resize, \
		.delete = &IAllocator_##N##_delete, \
	};

#define IAllocator_GENERATE_METHODS(N) \
	extern AllocatorAttr IAllocator_##N##_attr(Ptr this) { \
		return N##_attr(this); \
	} \
	extern Ptr IAllocator_##N##_new(Ptr this, usize size) { \
		return N##_new(this, size); \
	} \
	extern Ptr IAllocator_##N##_resize(Ptr this, Ptr buf, usize size) { \
		return N##_resize(this, buf, size); \
	} \
	extern void IAllocator_##N##_delete(Ptr this, Ptr buf) { \
		N##_delete(this, buf); \
	}

#if Allocator_PTRTAG
	#define IAllocator_GENERATE_UPCAST(N) \
		Allocator N##_upcast(Ptr this) { \
			return (Allocator){ptrtag(this, IAllocator_##N##_ID)}; \
		}

	#define IAllocator_REGISTER(N) INTERFACE_REGISTER(IAllocator, N)
	#define IAllocator_REGISTER_KNOWN(N) INTERFACE_REGISTER_KNOWN(IAllocator, N)

#else
	#define IAllocator_GENERATE_UPCAST(N) \
		Allocator N##_upcast(Ptr this) { \
			return (Allocator){.this=this,.iface=&IAllocator_##N}; \
		}

	#define IAllocator_REGISTER(N)
	#define IAllocator_REGISTER_KNOWN(N)

#endif

#define IAllocator_GENERATE_(N, REGISTER) \
	IAllocator_GENERATE_METHODS(N) \
	IAllocator_GENERATE_INTERFACE(N) \
	REGISTER(N) \
	IAllocator_GENERATE_UPCAST(N)

#define IAllocator_GENERATE(N) IAllocator_GENERATE_(N, IAllocator_REGISTER)
#define IAllocator_GENERATE_KNOWN(N) IAllocator_GENERATE_(N, IAllocator_REGISTER_KNOWN)
