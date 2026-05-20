typedef STRUCTDECL(AsyncRT);

typedef struct {
	Ptr value;
} AsyncTask;

typedef struct {
	Ptr value;
} AsyncFuture;

typedef enum : usize {
	AsyncIntent_SUSPEND,
	AsyncIntent_FINISH,
	AsyncIntent_PANIC,
} AsyncIntent;

typedef union {
	AsyncTask next;
	AsyncIntent intent;
} AsyncResult;

#define AsyncResult_SUSPEND ((AsyncResult){.intent=AsyncIntent_SUSPEND})
#define AsyncResult_FINISH ((AsyncResult){.intent=AsyncIntent_FINISH})
#define AsyncResult_PANIC ((AsyncResult){.intent=AsyncIntent_PANIC})

#include "AsyncFuture.h"
#include "AsyncTask.h"
#include "AsyncRT.h"
#include "AsyncFn.h"
