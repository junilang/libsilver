#define XS \
	X(Yield) /* async task finished without result */ \
	X(Resume) /* async task resumes parent task */ \
	X(Call) /* async task calls child task */ \
	X(Suspend) /* async task requests to be requeued */ \

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
	X(Unknown) \
	X(FutureInvalidate) \
	X(FutureGetError) \
	X(FutureSetError) \
	X(FutureSetTask) \
	X(FutureWait) \
	X(FutureResolve) \
	X(Aux)

	typedef u8 AsyncRes_Origin;
	enum : AsyncRes_Origin {
		#define X(N) AsyncRes_Origin_##N,
			XS
		#undef X
	};

	const String AsyncRes_Origin_repr[] = {
		#define X(N) [AsyncRes_Origin_##N] = String_INIT("AsyncRes_Origin_"#N),
			XS
		#undef X
	};

#undef XS

#define AsyncRes_Origin_AUX AsyncRes_Origin_Aux

#define XS \
	X(AsyncIntent) /* AsyncIntent is class=0 */ \
	X(AsyncWaitRes) \
	X(AsyncFutureRes) \
	X(AlcRes) \
	X(Pthread) \
	X(Unknown) \
	X(Aux)

	typedef u8 AsyncRes_Class;
	enum : AsyncRes_Class {
		#define X(N) AsyncRes_Class_##N,
			XS
		#undef X
	};

	const String AsyncRes_Class_repr[] = {
		#define X(N) [AsyncRes_Class_##N] = String_INIT("AsyncRes_Class_"#N),
			XS
		#undef X
	};

#undef XS

#define AsyncRes_Class_AUX AsyncRes_Class_Aux

typedef u16 AsyncRes_Code;

typedef u32 AsyncRes; enum {
	FIELD_DEF(AsyncRes_Code, 16),
	FIELD_DEF(AsyncRes_Class, 8),
	FIELD_DEF(AsyncRes_Origin, 8)
};

#define AsyncRes_WIDTH 32
constexpr u8 AsyncRes_width = AsyncRes_WIDTH;

constexpr u8 AsyncRes_Code_width = FIELD_WIDTH(AsyncRes_Code);
constexpr auto AsyncRes_Code_max = FIELD_MAX(AsyncRes_Code);

constexpr u8 AsyncRes_Class_width = FIELD_WIDTH(AsyncRes_Class);
constexpr auto AsyncRes_Class_max = FIELD_MAX(AsyncRes_Class);

constexpr u8 AsyncRes_Origin_width = FIELD_WIDTH(AsyncRes_Origin);
constexpr auto AsyncRes_Origin_max = FIELD_MAX(AsyncRes_Origin);

constexpr AsyncRes AsyncRes_Ok = 0;

AsyncRes_Origin AsyncRes_origin(AsyncRes error) {
	return FIELD_GET(AsyncRes_Origin, error);
}

AsyncRes_Class AsyncRes_class(AsyncRes error) {
	return FIELD_GET(AsyncRes_Origin, error);
}

AsyncRes_Code AsyncRes_code(AsyncRes error) {
	return FIELD_GET(AsyncRes_Code, error);
}
