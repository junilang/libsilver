Ptr Malloc_new(Ptr this, usize size) {
	#if Allocator_SAFE
		if (!size) PANIC("size = 0");
	#endif

	Ptr mem = malloc(size);

	#if Allocator_SAFE
		if (!mem) PANIC("malloc failed");
	#endif

	return mem;
}

Ptr Malloc_resize(Ptr this, Ptr buf, usize size) {
	#if Allocator_SAFE
		if (!size) PANIC("size = 0");
	#endif

	Ptr mem = realloc(buf, size);

	#if Allocator_SAFE
		if (!mem) PANIC("realloc failed");
	#endif

	return mem;
}

void Malloc_delete(Ptr this, Ptr buf) {
	#if Allocator_SAFE
		if (!buf) PANIC("buf = null");
	#endif

	free(buf);
}

AllocatorAttr Malloc_attr(Ptr this) {
	return FLAG(AllocatorAttr, THREADSAFE);
}

IAllocator_GENERATE_KNOWN(Malloc)

#define Malloc Malloc_upcast(nullptr)

Ptr AlignedMalloc_new(Ptr this, ualign align, usize size) {
	#if AlignedAllocator_SAFE
		if (stdc_count_ones(align) != 1)
			PANIC("align is not power of 2");

		if (!size)
			PANIC("size = 0")
	#endif

	const usize align_bit = (1ull << align);
	const usize mask = align_bit - 1;
	size = ((size + mask) & (~mask)) + align + sizeof(ualign);

	usize mem = (usize)malloc(size);

	#if AlignedAllocator_SAFE
		if (!mem) PANIC("malloc failed");
	#endif

	Ptr buf = (Ptr)((mem + align_bit) & (~mask));

	return buf;
}

Ptr AlignedMalloc_resize(Ptr this, ualign align, Ptr buf, usize size) {

}

Ptr AlignedMalloc_delete(Ptr this, ualign align, Ptr buf) {

}

AlignedAllocatorAttr AlignedMalloc_attr(Ptr this) {
	return FLAG(AlignedAllocatorAttr, THREADSAFE);
}

IAlignedAllocator_GENERATE_KNOWN(AlignedMalloc)
