#define IAlignedAllocator_GENERATE_INTERFACE(N) \
	const IAlignedAllocator IAlignedAllocator_##N = { \
		.attr = &IAlignedAllocator_##N##_attr, \
		.new = &IAlignedAllocator_##N##_new, \
		.resize = &IAlignedAllocator_##N##_resize, \
		.delete = &IAlignedAllocator_##N##_delete, \
	};

#define IAlignedAllocator_GENERATE_METHODS(N) \
	extern AlignedAllocatorAttr IAlignedAllocator_##N##_attr(Ptr this) { \
		return N##_attr(this); \
	} \
	extern Ptr IAlignedAllocator_##N##_new(Ptr this, ualign align, usize size) { \
		return N##_new(this, align, size); \
	} \
	extern Ptr IAlignedAllocator_##N##_resize(Ptr this, ualign align, Ptr buf, usize size) { \
		return N##_resize(this, align, buf, size); \
	} \
	extern void IAlignedAllocator_##N##_delete(Ptr this, ualign align, Ptr buf) { \
		N##_delete(this, align, buf); \
	}

#if AlignedAllocator_PTRTAG
	#define IAlignedAllocator_GENERATE_UPCAST(N) \
		AlignedAllocator N##_upcast(Ptr this) { \
			return (AlignedAllocator){ptrtag(this, IAlignedAllocator_##N##_ID)}; \
		}

	#define IAlignedAllocator_REGISTER(N) INTERFACE_REGISTER(IAlignedAllocator, N)
	#define IAlignedAllocator_REGISTER_KNOWN(N) INTERFACE_REGISTER_KNOWN(IAlignedAllocator, N)

#else
	#define IAlignedAllocator_GENERATE_UPCAST(N) \
		AlignedAllocator N##_upcast(Ptr this) { \
			return (AlignedAllocator){.this=this,.iface=&IAlignedAllocator_##N}; \
		}

	#define IAlignedAllocator_REGISTER(N)
	#define IAlignedAllocator_REGISTER_KNOWN(N)

#endif

#define IAlignedAllocator_GENERATE_(N, REGISTER) \
	IAlignedAllocator_GENERATE_METHODS(N) \
	IAlignedAllocator_GENERATE_INTERFACE(N) \
	REGISTER(N) \
	IAlignedAllocator_GENERATE_UPCAST(N)

#define IAlignedAllocator_GENERATE(N) IAlignedAllocator_GENERATE_(N, IAlignedAllocator_REGISTER)
#define IAlignedAllocator_GENERATE_KNOWN(N) IAlignedAllocator_GENERATE_(N, IAlignedAllocator_REGISTER_KNOWN)
