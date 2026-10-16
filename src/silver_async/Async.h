typedef STRUCTDECL(AsyncTask);
typedef STRUCTDECL(AsyncResult);

typedef enum : u8 {
	AsyncIntent_Yield, // async task finished without result
	AsyncIntent_Resume, // async task resumes parent task
	AsyncIntent_Call, // async task calls child task
	AsyncIntent_Suspend, // async task requests to be requeued
	AsyncIntent_Error // async task error
} AsyncIntent;

enum {
	AsyncError_Origin_None,
	AsyncError_Origin_Unknown,
	AsyncError_Origin_Submit,
	AsyncError_Origin_FutureResolve,
	AsyncError_Origin_FutureWait,
	AsyncError_Origin_Aux, // user errors
};

enum {
	AsyncError_Class_None,
	AsyncError_Class_Unknown,
	AsyncError_Class_AlcRes,
	AsyncError_Class_AsyncWaitRes,
	AsyncError_Class_AsyncFutureError,
	AsyncError_Class_Aux,
};

typedef u32 AsyncError; enum {
	FIELD_DEF(AsyncError_Code, 16),
	FIELD_DEF(AsyncError_Class, 8),
	FIELD_DEF(AsyncError_Origin, 8)
};


constexpr u8 AsyncError_width = 32;

constexpr u8 AsyncError_Code_width = FIELD_WIDTH(AsyncError_Code);
constexpr auto AsyncError_Code_max = FIELD_MAX(AsyncError_Code);

constexpr u8 AsyncError_Class_width = FIELD_WIDTH(AsyncError_Class);
constexpr auto AsyncError_Class_max = FIELD_MAX(AsyncError_Class);

constexpr u8 AsyncError_Origin_width = FIELD_WIDTH(AsyncError_Origin);
constexpr auto AsyncError_Origin_max = FIELD_MAX(AsyncError_Origin);

constexpr AsyncError AsyncError_none = 0;
