typedef STRUCTDECL(AsyncTask);

typedef enum : u8 {
	AsyncIntent_Yield, // async task finished without result
	AsyncIntent_Resume, // async task finished and requests to resume parent task
	AsyncIntent_Call, // async task calls child task
	AsyncIntent_Suspend, // async task requests to be requeued
	AsyncIntent_Error // async task error
} AsyncIntent;
