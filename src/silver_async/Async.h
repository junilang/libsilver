typedef STRUCTDECL(AsyncTask);
typedef STRUCTDECL(AsyncResult);

#define XS \
	X(Yield) /* async task finished without result */ \
	X(Resume) /* async task resumes parent task */ \
	X(Call) /* async task calls child task */ \
	X(Suspend) /* async task requests to be requeued */ \
	X(Error) /* async task error */

	typedef enum : u8 {
		#define X(N) AsyncIntent_##N,
			XS
		#undef X
	} AsyncIntent;

	const String AsyncIntent_repr[] = {
		#define X(N) [AsyncIntent_##N] = String_INIT("AsyncIntent_"#N),
			XS
		#undef X
	};

#undef XS

#define XS \
	X(None) \
	X(Unknown) \
	X(FutureInvalidate) \
	X(FutureGetError) \
	X(FutureSetError) \
	X(FutureSetTask) \
	X(FutureWait) \
	X(FutureResolve) \
	X(Aux)

	typedef enum : u8 {
		#define X(N) AsyncError_Origin_##N,
			XS
		#undef X
	} AsyncError_Origin;

	const String AsyncError_Origin_repr[] = {
		#define X(N) [AsyncError_Origin_##N] = String_INIT("AsyncError_Origin_"#N),
			XS
		#undef X
	};

#undef XS


#define XS \
	X(None) \
	X(Unknown) \
	X(AlcRes) \
	X(AsyncWaitRes) \
	X(AsyncFutureRes) \
	X(Aux)

	typedef enum : u8 {
		#define X(N) AsyncError_Class_##N,
			XS
		#undef X
	} AsyncError_Class;

	const String AsyncError_Class_repr[] = {
		#define X(N) [AsyncError_Class_##N] = String_INIT("AsyncError_Class_"#N),
			XS
		#undef X
	};

#undef XS

typedef u16 AsyncError_Code;

typedef u32 AsyncError; enum {
	FIELD_DEF(AsyncError_Code, 16),
	FIELD_DEF(AsyncError_Class, 8),
	FIELD_DEF(AsyncError_Origin, 8)
};

#define AsyncError_WIDTH 32
constexpr u8 AsyncError_width = AsyncError_WIDTH;

constexpr u8 AsyncError_Code_width = FIELD_WIDTH(AsyncError_Code);
constexpr auto AsyncError_Code_max = FIELD_MAX(AsyncError_Code);

constexpr u8 AsyncError_Class_width = FIELD_WIDTH(AsyncError_Class);
constexpr auto AsyncError_Class_max = FIELD_MAX(AsyncError_Class);

constexpr u8 AsyncError_Origin_width = FIELD_WIDTH(AsyncError_Origin);
constexpr auto AsyncError_Origin_max = FIELD_MAX(AsyncError_Origin);

constexpr AsyncError AsyncError_none = 0;

AsyncError_Origin AsyncError_origin(AsyncError error) {
	return FIELD_GET(AsyncError_Origin, error);
}

AsyncError_Class AsyncError_class(AsyncError error) {
	return FIELD_GET(AsyncError_Origin, error);
}

AsyncError_Code AsyncError_code(AsyncError error) {
	return FIELD_GET(AsyncError_Code, error);
}
