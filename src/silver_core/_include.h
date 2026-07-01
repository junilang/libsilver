#if !LIBSILVER_CORE_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_CORE_INCLUDE
	#define LIBSILVER_CORE_INCLUDE true

	#include "types.h"
	#include "panic.h"
	#include "util.h"
	#include "flag.h"
	#include "field.h"
	#include "enum.h"
	#include "interface.h"

	#include "memory.h"
	#include "chkdop.h"
	#include "sizeutil.h"
	#include "ptrtag.h"
	#include "lptrtag.h"
	#include "chars.h"

	#include "hash/hash_crc32_x86.h"
	#include "hash/hash_fnv1a.h"
	#include "hash/hash.h"

	#include "outstream/OutStream.h"
	#include "printable/Printable.h"

	#include "outstream/VoidOutStream.c"
	#include "outstream/BufferOutStream.c"
	#include "outstream/BufferedOutStream.c"

	#include "String.h"
	#include "SmallString.h"

	#include "IntFmt.c"
	#include "print.h"

	#include "allocator/Alc.h"
	#include "allocator/StaticAlc.c"

#endif
