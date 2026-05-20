#if !LIBSILVER_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_INCLUDE

	#include "util.h"
	#include "enum.h"
	#include "flag.h"
	#include "types.h"
	#include "panic.h"
	#include "usize.h"
	#include "hash.h"

	#include "whitespace.h"

	#include "ptrtag.h"
	#include "lptrtag.h"

	#include "interface.h"

	#include "outstream/OutStream.h"
	#include "printable/Printable.h"

	#include "String.h"
	#include "VString.h"

	#include "print.h"

	#include "allocator/Allocator.h"

	#include "Vec.h"
	#include "Vec32.h"

	#include "trie/Trie.h"

	#include "Rc.h"
	#include "Arc.h"

	#include "Spinlock.h"

	#include "async/Async.h"


	#include "outstream/FileOutStream.c"

	#include "allocator/Malloc.c"


	#include "async/DummyAsync.c"
	#include "weaver/Weaver.c"

	#include "SilverTest.c"

	#define LIBSILVER_INCLUDE true

#endif
