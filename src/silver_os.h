#if __linux__
	#include "silver_linux/_include.h"

#elif _WIN32
	#include "silver_nt/_include.h"

#else
	#error "unsupported system"

#endif
