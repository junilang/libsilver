#if !LIBSILVER_CORE_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_CORE_INCLUDE
	#define LIBSILVER_CORE_INCLUDE true

	#include "types.h"
	#include "atomic_types.h"
	#include "atomic.h"

	#include "util.h"
	#include "flag.h"
	#include "field.h"
	#include "enum.h"
	#include "iface.h"
	#include "cpu.h"

	#include "memory.h"
	#include "chkdop.h"
	#include "sizeutil.h"

	#include "panic.h"

	#include "ptrtag.h"
	#include "chars.h"
	#include "intfmt.h"

	#include "outstream/OutStream.h"
	#include "printable/Printable.h"
	#include "printable/intfmt.h"

	#include "outstream/VoidOutStream.c"
	#include "outstream/BufferOutStream.c"
	#include "outstream/BufferedOutStream.c"

	#include "String.h"

	//#include "VString.h"

	#include "print.h"

	#include "allocator/Alc.h"
	#include "allocator/StaticAlc.c"

#endif
