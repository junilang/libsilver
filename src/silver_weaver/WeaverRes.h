enum : AsyncRes_Origin {
	AsyncRes_Origin_WeaverBoot = AsyncRes_Origin_AUX,
	AsyncRes_Origin_WeaverJoin,
	AsyncRes_Origin_WeaverWake,
	AsyncRes_Origin_WeaverSubmit,
	AsyncRes_Origin_WeaverInit,
	AsyncRes_Origin_WeaverCreate,
	AsyncRes_Origin_WeaverDeinit,
	AsyncRes_Origin_WeaverDestroy,
	AsyncRes_Origin_WeaverAux,
};

#undef AsyncRes_Origin_AUX
#define AsyncRes_Origin_AUX AsyncRes_Origin_WeaverAux

enum : AsyncRes_Class {
	AsyncRes_Class_WeaverRes = AsyncRes_Class_AUX,
	AsyncRes_Class_WeaverAux
};

#undef AsyncRes_Class_AUX
#define AsyncRes_Class_AUX AsyncRes_Class_WeaverAux

enum : u8 {
	WeaverRes_ErrTooManyTasks
};
