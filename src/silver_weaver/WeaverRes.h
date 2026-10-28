enum : u8 {
	AsyncRes_Origin_WeaverBoot = AsyncRes_Origin_Aux,
	AsyncRes_Origin_WeaverJoin,
	AsyncRes_Origin_WeaverWake,
	AsyncRes_Origin_WeaverSubmit,
	AsyncRes_Origin_WeaverAux,
};

enum : u8 {
	AsyncRes_Class_WeaverRes = AsyncRes_Class_Aux,
	AsyncRes_Class_WeaverAux
};

enum : u8 {
	WeaverRes_ErrTooManyTasks
};
