#if !LIBSILVER_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_INCLUDE
	#define LIBSILVER_INCLUDE true

	// macro utilities
	#include "util.h"
	#include "flag.h"
	#include "field.h"
	#include "enum.h"
	#include "panic.h"
	#include "interface.h"

	// core features
	#include "types.h"
	#include "memory.h"

	#include "hash.h"
	#include "ptrtag.h"
	#include "lptrtag.h"

	#include "outstream/OutStream.h"

	#include "printable/Printable.h"

	#include "String.h"

	#include "outstream/BufferOutStream.c"
	#include "outstream/RawFileOutStream.c"

	#include "IntFmt.c"

	#include "print.h"

	#include "SilverTest.c"

	#if 0
	#include "whitespace.h"

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
