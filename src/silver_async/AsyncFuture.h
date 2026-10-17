#ifndef AsyncFuture_SAFE
	#define AsyncFuture_SAFE LIBSILVER_SAFE
#endif

typedef enum : u8 {
	AsyncFutureRes_Ok,
	AsyncFutureRes_ErrResolved,
	AsyncFutureRes_ErrUnderflow,
} AsyncFutureRes;

typedef u32 AsyncFutureInfo; enum {
	// lock set when writing error or result during resolution
	FLAG_DEF(AsyncFuture_Lock),
	// thread(s) is waiting on the state
	FLAG_DEF(AsyncFuture_Waiting),
	// task intent 2 bits is enough for
	// 00=no task (Yield), 01=Resume, 10=Suspend, 11=Call
	FIELD_DEF(AsyncFuture_Intent, 2),
	// future is resolved
	FLAG_DEF(AsyncFuture_Resolved),
	// future is resolved with error
	FLAG_DEF(AsyncFuture_Error),
	// an error occured during interaction with future (error is not set)
	FLAG_DEF(AsyncFuture_Invalid),
	FIELD_DEF_UNTIL(AsyncFuture_Count, 32),
};


constexpr u8 AsyncFuture_Count_underflowbits = 2;

constexpr AsyncFutureInfo AsyncFuture_Count_mask = FIELD_MASK(AsyncFuture_Count);
constexpr AsyncFutureInfo AsyncFuture_Count_one = FIELD_SET(AsyncFuture_Count, 1);
constexpr AsyncFutureInfo AsyncFuture_Count_zero =
	FIELD_MASK_UPPER(AsyncFuture_Count, AsyncFuture_Count_underflowbits)
;

constexpr AsyncFutureInfo AsyncFuture_Lock = FLAG(AsyncFuture_Lock);

constexpr auto AsyncFuture_maxtriggers =
	FIELD_MAX(AsyncFuture_Count) >> AsyncFuture_Count_underflowbits
;

constexpr AsyncFutureInfo AsyncFuture_Count_single =
	AsyncFuture_Count_zero | AsyncFuture_Count_one
;

constexpr AsyncFutureInfo AsyncFutureInfo_resolved =
	FLAG(AsyncFuture_Resolved)
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
	constexpr auto error_future =
		FIELD(AsyncError_Class, AsyncFutureRes) |
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

	AsyncError error;

	AsyncFutureInfo info = atom_sub(&this->info, AsyncFuture_Count_one, atom_sync);
	// if future is already resolved, invalidate future
	if (info & FLAG(AsyncFuture_Resolved)) {
		error = error_future |
			FIELD_SET(AsyncError_Code, AsyncFutureRes_ErrResolved)
		;
		goto invalidate;
	}

	auto const count = info & AsyncFuture_Count_mask;

	// if count is not 1 return
	if (count > AsyncFuture_Count_single)
		return AsyncIntent_Yield;

	// check if count underflowed
	if (count < AsyncFuture_Count_single) {
		error = error_future |
			FIELD_SET(AsyncError_Code, AsyncFutureRes_ErrUnderflow)
		;
		goto invalidate;
	}

	// resolve future
	info = atom_or(&this->info, AsyncFutureInfo_resolved, atom_sync);

	AsyncIntent intent = FIELD_GET(AsyncFuture_Intent, info);
	// if intent for resuming task is set, return the task
	// in the result field
	if (intent) {
		// wait for write lock
		while (info & FLAG(AsyncFuture_Lock)) {
			CPU_YIELD;
			info = atom_get(&this->info, atom_acq);
		}
		result->task = ((AsyncFutureTask*)this)->task;
	}

	// wake up threads waiting on the state
	if (info & FLAG(AsyncFuture_Waiting)) {
		auto res = Async_wake32(&this->info, Async_wakeall);
		if (res) {
			error = error_wake |
				FIELD_SET(AsyncError_Code, res);
			;
			goto return_error;
		}
	}

	return intent;

	// if an error occured during resolution, invalidate the future
	invalidate:;
	info = atom_or(&this->info, AsyncFutureInfo_invalid, atom_sync);

	// if future is waiting and not resolved yet, wake up waiting threads
	if ((info & FLAGS(AsyncFuture, Resolved, Waiting)) ^ FLAG(AsyncFuture_Waiting)) {
		auto res = Async_wake32(&this->info, Async_wakeall);
		if (res) error = error_unknown;
	}

	return_error:;
	result->error = error;
	return AsyncIntent_Error;

	#undef XERROR
}

AsyncIntent AsyncFuture_resolve_error(
	AsyncFuture *this, AsyncError future_error, AsyncResult *result
) {
	constexpr auto error_future =
		FIELD(AsyncError_Class, AsyncFutureRes) |
		FIELD(AsyncError_Origin, FutureResolveError)
	;

	AsyncError error;

	retry:;
	// set error flag and lock future for writing error value
	AsyncFutureInfo info = atom_or(&this->info,
		AsyncFutureInfo_error | AsyncFuture_Lock,
		atom_sync
	);

	// if future was already resolved invalidate it
	if (info & FLAG(AsyncFuture_Resolved)) {
		error = error_future |
			FIELD_SET(AsyncError_Code, AsyncFutureRes_ErrResolved)
		;

		// if we set the lock clear it
		if (!(info & AsyncFuture_Lock))
			info = atom_and(&this->info, ~AsyncFuture_Lock, atom_sync);

		goto invalidate;
	}

	// if lock flag was already set, wait for it to unlock
	if (info & AsyncFuture_Lock) {
		do {
			CPU_YIELD;
			info = atom_get(&this->info, atom_acq);
		} while (info & AsyncFuture_Lock);
		goto retry;
	}

	// we have a valid write lock for the future
	// and can set the error
	this->error = future_error;
	info = atom_and(&this->info, AsyncFuture_Lock, atom_sync);

	return AsyncIntent_Yield;

	invalidate:;
	info = atom_or(&this->info, AsyncFutureInfo_invalid, atom_sync);

	[[maybe_unused]]
	return_error:;
	result->error = error;
	return AsyncIntent_Error;
}

void AsyncFuture_init(AsyncFuture *this, u32 triggers) {
	#if AsyncFuture_SAFE
		if (triggers > AsyncFuture_maxtriggers)
			PANIC("too many triggers");

		if (triggers == 0)
			PANIC("triggers needs to be set at at least 1");
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Count_zero | (AsyncFuture_Count_one * triggers)
	;

	#if AsyncFuture_SAFE
		info = atom_exchg(&this->info, info, atom_sync);
		if (info)
			PANIC("future not zeroed");
	#else
		this->info = info;
	#endif
}

void AsyncFutureTask_init(
	AsyncFutureTask *this, u32 triggers, AsyncTask task, AsyncIntent intent
) {
	#if AsyncFuture_SAFE
		if (triggers > AsyncFuture_maxtriggers)
			PANIC("too many triggers");

		if (triggers == 0)
			PANIC("triggers needs to be set at at least 1");

		if (intent > FIELD_MAX(AsyncFuture_Intent))
			PANIC("intent out of range")
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Count_zero | (AsyncFuture_Count_one * triggers) |
		FIELD_SET(AsyncFuture_Intent, intent)
	;

	#if AsyncFuture_SAFE
		info = atom_exchg(&this->header.info, info, atom_sync);
		if (info)
			PANIC("future not zeroed");
	#else
		this->info = info;
	#endif

	this->task = task;
}

AsyncError AsyncFuture_wait(AsyncFuture *this) {


	// set waiting flag
	AsyncFutureInfo info = atom_or(&this->info, FLAG(AsyncFuture_Waiting), atom_sync);

	// if already resolved
	if (info & FLAG(AsyncFuture_Resolved)) {

	}

}
