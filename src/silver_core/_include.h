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

	#include "String.h"

	#include "memory.h"
	#include "chkdop.h"
	#include "sizeutil.h"

	#include "panic.h"

	#include "ptrtag.h"
	#include "chars.h"
	#include "intfmt.h"

	#include "sink/Sink.h"
	#include "fmt/Fmt.h"
	#include "fmt/intfmt.h"

	#include "sink/BufferSink.c"
	#include "sink/BufferedSink.c"

	#include "print.h"

	#include "alc/Alc.h"
	#include "alc/BumpAlc.c"

#endif
