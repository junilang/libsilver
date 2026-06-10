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
	#include "chars.h"

	#include "outstream/OutStream.h"
	#include "printable/Printable.h"

	#include "outstream/BufferOutStream.c"
	#include "outstream/RawFileOutStream.c"

	#include "String.h"
	#include "IntFmt.c"
	#include "print.h"

#endif
