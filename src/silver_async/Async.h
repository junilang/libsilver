typedef STRUCTDECL(AsyncTask);
typedef STRUCTDECL(AsyncResult);

typedef enum : u8 {
	AsyncIntent_YIELD,
	AsyncIntent_CALL,
	AsyncIntent_RESUME,
	AsyncIntent_SUSPEND
} AsyncIntent;
