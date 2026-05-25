typedef STRUCTDECL(AsyncRT);

typedef struct {
	Ptr value;
} AsyncTask;

typedef struct {
	Ptr value;
} AsyncFuture;

typedef enum : usize {
	AsyncIntent_YIELD,
	AsyncIntent_FINISH
} AsyncIntent;

typedef union {
	AsyncTask next;
	AsyncIntent intent;
} AsyncResult;

#define AsyncResult_YIELD ((AsyncResult){.intent=AsyncIntent_YIELD})
#define AsyncResult_FINISH ((AsyncResult){.intent=AsyncIntent_FINISH})
