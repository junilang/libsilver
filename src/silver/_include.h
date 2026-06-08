#if !LIBSILVER_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_INCLUDE
	#define LIBSILVER_INCLUDE true

	// macro utilities
	#include "util.h"
	#include "flag.h"
	#include "enum.h"
	#include "panic.h"
	#include "interface.h"

	// core features
	#include "types.h"
	#include "atomic_types.h"
	#include "atomic.h"
	#include "memory.h"
	#include "hash.h"
	#include "ptrtag.h"
	#include "lptrtag.h"

	#include "outstream/OutStream.h"

	#include "printable/Printable.h"

	#include "String.h"

	#include "outstream/FileOutStream.c"
	#include "outstream/BufferOutStream.c"

	#include "AnyInt.c"

	#include "print.h"

	#include "SilverTest.c"

	#if 0
	#include "panic.h"
	#include "usize.h"
	#include "hash.h"

	#include "whitespace.h"

	#include "ptrtag.h"
	#include "lptrtag.h"

	#include "interface.h"

	#include "outstream/OutStream.h"
	#include "outstream/FileOutStream.c"
	#include "outstream/BufferOutStream.c"

	#include "printable/Printable.h"

	#include "allocator/Allocator.h"
	#include "allocator/AlignedAllocator.h"

	#include "VString.h"

	#include "Vec.h"
	#include "Vec32.h"

	#include "trie/Trie.h"

	#include "Rc.h"
	#include "Arc.h"


	#include "allocator/Malloc.c"

	#endif

	//#include "SilverTest.c"

#endif
