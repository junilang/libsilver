typedef STRUCTDECL(AsyncTask);

typedef enum : u8 {
	AsyncIntent_Yield,
	AsyncIntent_Resume,
	AsyncIntent_Call,
	AsyncIntent_Suspend
} AsyncIntent;
