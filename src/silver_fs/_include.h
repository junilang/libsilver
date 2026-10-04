#if !LIBSILVER_FS_DEPENDS
	#include "_depends.h"
#endif

#if !LIBSILVER_FS_INCLUDE
	#define LIBSILVER_FS_INCLUDE true

	#include "FsRes.h"
	#include "Fs.h"

	#if __linux__
		#include "Fs_linux.h"
	#elif _WIN32
		#include "Fs_nt.h"
	#else
		#error "unsupported platform"
	#endif

#endif
