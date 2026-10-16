#ifndef AsyncFuture_SAFE
	#define AsyncFuture_SAFE LIBSILVER_SAFE
#endif

typedef enum : u8 {
	AsyncFutureError_None,
	AsyncFutureError_NotReady,
	AsyncFutureError_InvalidState,
	AsyncFutureError_AlreadyResolved,
	AsyncFutureError_CountUnderflow,
} AsyncFutureError;

typedef u32 AsyncFutureInfo; enum {
	FLAG_DEF(AsyncFuture_Ready),
	FLAG_DEF(AsyncFuture_Waiting),
	FLAG_DEF(AsyncFuture_Task),
	FLAG_DEF(AsyncFuture_Resolved),
	FLAG_DEF(AsyncFuture_Error),
	FLAG_DEF(AsyncFuture_Invalid),
	FIELD_DEF_UNTIL(AsyncFuture_Count, 32),
};

constexpr u8 AsyncFuture_Count_underflowbits = 4;

constexpr AsyncFutureInfo AsyncFuture_Count_mask = FIELD_MASK(AsyncFuture_Count);
constexpr AsyncFutureInfo AsyncFuture_Count_one = FIELD_SET(AsyncFuture_Count, 1);
constexpr AsyncFutureInfo AsyncFuture_Count_zero =
	FIELD_MASK_UPPER(AsyncFuture_Count, AsyncFuture_Count_underflowbits)
;

constexpr auto AsyncFuture_maxtriggers =
	FIELD_MAX(AsyncFuture_Count) >> AsyncFuture_Count_underflowbits
;

constexpr AsyncFutureInfo AsyncFuture_Count_single =
	AsyncFuture_Count_zero | AsyncFuture_Count_one
;

constexpr AsyncFutureInfo AsyncFutureInfo_resolved =
	FIELD_MASK(AsyncFuture_Count) | FLAG(AsyncFuture_Resolved)
;

constexpr AsyncFutureInfo AsyncFutureInfo_error =
	AsyncFutureInfo_resolved | FLAG(AsyncFuture_Error)
;

constexpr AsyncFutureInfo AsyncFutureInfo_invalid =
	AsyncFutureInfo_resolved | FLAG(AsyncFuture_Invalid)
;

typedef struct {
	_Atomic u32_lf info;
	AsyncError error;
} AsyncFuture;

typedef struct {
	AsyncFuture header;
	AsyncTask task;
} AsyncFutureTask;

AsyncIntent AsyncFuture_resolve(AsyncFuture *this, AsyncResult *result) {
	constexpr auto error_base =
		FIELD(AsyncError_Class, AsyncFutureError) |
		FIELD(AsyncError_Origin, FutureResolve)
	;

	constexpr auto error_wake =
		FIELD(AsyncError_Class, AsyncWaitRes) |
		FIELD(AsyncError_Origin, FutureResolve)
	;

	constexpr auto error_unknown =
		FIELD(AsyncError_Class, Unknown) |
		FIELD(AsyncError_Origin, FutureResolve)
	;

	#define XERROR(err) (error_base | FIELD_SET(AsyncError_Code, AsyncFutureError_##err))

	AsyncError error;

	AsyncFutureInfo info = atom_sub(&this->info, AsyncFuture_Count_one, atom_sync);
	// if future is already resolved
	if (info & FLAG(AsyncFuture_Resolved)) {
		error = XERROR(AlreadyResolved);
		goto return_error;
	}

	if (!(info & FLAG(AsyncFuture_Ready))) {
		error = XERROR(NotReady);
		goto invalidate;
	}

	auto const count = info & AsyncFuture_Count_mask;

	// if count is not 1
	if (count > AsyncFuture_Count_single)
		return AsyncIntent_Yield;

	// check count underflow
	if (count < AsyncFuture_Count_single) {
		error = XERROR(CountUnderflow);
		goto invalidate;
	}

	info = atom_or(&this->info, AsyncFutureInfo_resolved, atom_sync);
	// ensure ready flag is set
	if (!(info & FLAG(AsyncFuture_Ready))) {
		error = XERROR(NotReady);
		goto invalidate;
	}

	// wake up waiting threads on this future
	if (info & FLAG(AsyncFuture_Waiting)) {
		auto res = Async_wake32(&this->info, Async_wakeall);
		if (res) {
			error = error_wake | FIELD_SET(AsyncError_Code, res);
			goto return_error;
		}
	}

	// return task
	if (info & FLAG(AsyncFuture_Task)) {
		result->task = ((AsyncFutureTask*)this)->task;
		return AsyncIntent_Resume;
	}

	return AsyncIntent_Yield;

	// if an error occured during resolution, invalidate the future
	invalidate:;
	info = atom_or(&this->info, AsyncFutureInfo_invalid, atom_sync);
	if (info & FLAG(AsyncFuture_Resolved))
		goto return_error;

	if (info & FLAG(AsyncFuture_Waiting)) {
		auto res = Async_wake32(&this->info, Async_wakeall);
		if (res) error = error_unknown;
	}

	return_error:;
	result->error = error;
	return AsyncIntent_Error;

	#undef XERROR
}

void AsyncFuture_init(AsyncFuture *this, u32 triggers) {
	#if AsyncFuture_SAFE
		if (triggers > AsyncFuture_maxtriggers)
			PANIC("too many triggers");

		if (triggers == 0)
			PANIC("triggers needs to be set at at least 1");
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Count_zero | (AsyncFuture_Count_one * triggers) |
		FLAG(AsyncFuture_Ready)
	;

	#if AsyncFuture_SAFE
		info = atom_exchg(&this->info, info, atom_sync);
		if (info)
			PANIC("future not zeroed");
	#else
		this->info = info;
	#endif
}

void AsyncFutureTask_init(AsyncFutureTask *this, AsyncTask task, u32 triggers) {
	#if AsyncFuture_SAFE
		if (triggers > AsyncFuture_maxtriggers)
			PANIC("too many triggers");

		if (triggers == 0)
			PANIC("triggers needs to be set at at least 1");
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Count_zero | (AsyncFuture_Count_one * triggers) |
		FLAG(AsyncFuture_Ready)
	;

	#if AsyncFuture_SAFE
		info = atom_exchg(&this->info, info, atom_sync);
		if (info)
			PANIC("future not zeroed");
	#else
		this->info = info;
	#endif
}

AsyncError AsyncFuture_wait(AsyncFuture *this) {

}
