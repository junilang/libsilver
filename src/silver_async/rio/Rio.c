#ifndef Rio_SAFE
	#define Rio_SAFE BUILD_SAFE
#endif

#define Rio_BACKEND_LINUX_IO_URING 1

#ifndef Rio_BACKEND
	#ifdef __linux__
		#define Rio_BACKEND Rio_BACKEND_LINUX_IO_URING

	#else
		#error "unsupported platform"

	#endif
#endif

#include "RioTask.h"
#include "RioQueue.h"

#if Rio_BACKEND == Rio_BACKEND_LINUX_IO_URING
	#include "linux_io_uring/Worker.h"

#endif

#include "Rio.h"

void Rio_submit(Ptr tasks, usize tasks_size) {
}
