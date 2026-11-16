ENUM_GENERATE(AsyncIntent, u8,
	Yield, // async task finished without result
	Resume, // async task resumes parent task
	Call, // async task calls child task
	Suspend // async task requests to be requeued
)

typedef u8 AsyncRes_Origin;
ENUM_GENERATE_NTD(AsyncRes_Origin, AsyncRes_Origin,
	Unknown,
	FutureInvalidate,
	FutureGetError,
	FutureSetError,
	FutureSetTask,
	FutureWait,
	FutureResolve,
	Aux
)

#define AsyncRes_Origin_AUX AsyncRes_Origin_Aux

typedef u8 AsyncRes_Class;
ENUM_GENERATE_NTD(AsyncRes_Class, AsyncRes_Class,
	AsyncIntent, // AsyncIntent is class=0
	AsyncWaitRes,
	AsyncFutureRes,
	AlcRes,
	Pthread,
	Unknown,
	Aux
)


#define AsyncRes_Class_AUX AsyncRes_Class_Aux

typedef u16 AsyncRes_Code;

typedef u32 AsyncRes; enum {
	FIELD_DEF(AsyncRes_Code, 16),
	FIELD_DEF(AsyncRes_Class, 8),
	FIELD_DEF(AsyncRes_Origin, 8),
	AsyncRes_width
};

static_assert(AsyncRes_width <= 32);

auto constexpr AsyncRes_Code_width = FIELD_WIDTH(AsyncRes_Code);
auto constexpr AsyncRes_Code_max = FIELD_MAX(AsyncRes_Code);

auto constexpr AsyncRes_Class_width = FIELD_WIDTH(AsyncRes_Class);
auto constexpr AsyncRes_Class_max = FIELD_MAX(AsyncRes_Class);

auto constexpr AsyncRes_Origin_width = FIELD_WIDTH(AsyncRes_Origin);
auto constexpr AsyncRes_Origin_max = FIELD_MAX(AsyncRes_Origin);

AsyncRes constexpr AsyncRes_Ok = 0;
AsyncRes constexpr AsyncRes_Err =
	FIELD(AsyncRes_Class, Unknown) | FIELD_SET(AsyncRes_Code, 1)
;

AsyncRes_Origin AsyncRes_origin(AsyncRes error) {
	return FIELD_GET_CAST(AsyncRes_Origin, error);
}

AsyncRes_Class AsyncRes_class(AsyncRes error) {
	return FIELD_GET_CAST(AsyncRes_Class, error);
}

AsyncRes_Code AsyncRes_code(AsyncRes error) {
	return FIELD_GET_CAST(AsyncRes_Code, error);
}

[[noreturn, gnu::noinline]]
void AsyncRes_panic_(AsyncRes res, ConstPtr func_name, ConstPtr header) {
	// TODO print res value
	String msg[] = {
		String_INIT(func_name),
		String_INIT(header)
	};
	panic_msg_(msg, _Countof(msg));
}

void AsyncRes_assert_(AsyncRes res, ConstPtr func_name, ConstPtr header) {
	if (res) AsyncRes_panic_(res, func_name, header);
}

#define AsyncRes_ASSERT(result) ASSERT_RES(AsyncRes, result)
