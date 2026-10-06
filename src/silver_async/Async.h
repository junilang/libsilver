typedef STRUCTDECL(AsyncTask);
typedef STRUCTDECL(AsyncResult);

typedef enum : u8 {
	AsyncIntent_Yield, // async task finished without result
	AsyncIntent_Resume, // async task resumes parent task
	AsyncIntent_Call, // async task calls child task
	AsyncIntent_Suspend, // async task requests to be requeued
	AsyncIntent_Error // async task error
} AsyncIntent;

typedef struct {
	usize value;
} AsyncError;
