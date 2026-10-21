#ifndef AsyncFuture_SAFE
	#define AsyncFuture_SAFE LIBSILVER_SAFE
#endif

// whether to wait for write lock when an operation
// returns AsyncFutureRes_IsError
#ifndef AsyncFuture_SYNCERROR
	#define AsyncFuture_SYNCERROR true
#endif

// whether to invalidate future on certain error conditions
// like setting an error twice, or underflowing triggers
#ifndef AsyncFuture_STRICT
	#define AsyncFuture_STRICT AsyncFuture_SAFE
#endif

// whether to use the 8 byte info header only for waiting
#ifndef AsyncFuture_WAIT8
	#define AsyncFuture_WAIT8 (!AsyncFuture_SAFE)
#endif

#define XS \
	X(Ok) \
	X(Timeout) \
	X(IsSet) \
	X(IsResolved) \
	X(IsError) \
	X(IsInvalid) \
	X(ErrUnderflow) \
	X(ErrUnknown)

	typedef enum : u8 {
		#define X(N) AsyncFutureRes_##N,
			XS
		#undef X
	} AsyncFutureRes;

	const String AsyncFutureRes_repr[] = {
		#define X(N) [AsyncFutureRes_##N] = String_INIT("AsyncFutureRes_"#N),
			XS
		#undef X
	};

#undef XS

typedef u32 AsyncFutureInfo; enum {
	// future is resolved
	FLAG_DEF(AsyncFuture_Resolved),
	// future is resolved with error
	FLAG_DEF(AsyncFuture_Error),
	// an error occured during interaction with future (error is not set)
	FLAG_DEF(AsyncFuture_Invalid),
	// lock set when thread is writing an error
	FLAG_DEF(AsyncFuture_ErrorLock),
	// lock set when thread is setting the task
	FLAG_DEF(AsyncFuture_TaskLock),

	// task intent, 2 bits is enough for
	// 00=no task (Yield), 01=Resume, 10=Suspend, 11=Call
	FIELD_DEF(AsyncFuture_Intent, 2),

	// number of threads waiting on the future
	FIELD_DEF_FROM(AsyncFuture_Waiting, 8, 10),

	// number of resolve calls needed to complete the future
	FIELD_DEF_UNTIL(AsyncFuture_Triggers, 32),
};

static_assert(AsyncFuture_Intent_FIELD_END < AsyncFuture_Triggers_FIELD);

#if AsyncFuture_WAIT8
	// ensure that the state header fits into a single byte
	static_assert(AsyncFuture_Intent_FIELD_END < 8);
#endif


constexpr AsyncFutureInfo AsyncFuture_ErrorLock = FLAG(AsyncFuture_ErrorLock);
constexpr AsyncFutureInfo AsyncFuture_TaskLock = FLAG(AsyncFuture_TaskLock);

constexpr AsyncFutureInfo AsyncFuture_Waiting_one = FIELD_SET(AsyncFuture_Waiting, 1);

constexpr AsyncFutureInfo AsyncFuture_Triggers_one = FIELD_SET(AsyncFuture_Triggers, 1);

// number of top bits in triggers field used for underflow detection
constexpr u8 AsyncFuture_Triggers_underflowbits = 2;

// the "zero" value has the top underflow bits set to ones
constexpr AsyncFutureInfo AsyncFuture_Triggers_zero =
	FIELD_MASK_UPPER(AsyncFuture_Triggers, AsyncFuture_Triggers_underflowbits)
;

// the treshold for detecting succesfull resolves and underflows
constexpr AsyncFutureInfo AsyncFuture_Triggers_single =
	AsyncFuture_Triggers_zero | AsyncFuture_Triggers_one
;

constexpr u32 AsyncFuture_maxtriggers =
	FIELD_MAX(AsyncFuture_Triggers) >> AsyncFuture_Triggers_underflowbits
;

typedef struct {
	_Atomic u32_lf info;
	AsyncError error;
	AsyncTask task;
} AsyncFuture;



void AsyncFuture_init(AsyncFuture *this, u32 triggers) {
	#if AsyncFuture_SAFE
		if (triggers > AsyncFuture_maxtriggers)
			PANIC("too many triggers");

		if (triggers == 0)
			PANIC("need at least one intent");
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Triggers_zero | (AsyncFuture_Triggers_one * triggers)
	;

	#if AsyncFuture_SAFE
		info = atom_exchg(&this->info, info, atom_sync);
		if (info)
			PANIC("future not zeroed");
	#else
		this->info = info;
	#endif
}



void AsyncFuture_inittask(
	AsyncFuture *this, u32 triggers, AsyncTask task, AsyncIntent intent
) {
	#if AsyncFuture_SAFE
		if (triggers > AsyncFuture_maxtriggers)
			PANIC("too many triggers");

		if (triggers == 0)
			PANIC("need at least one trigger");

		if (intent > FIELD_MAX(AsyncFuture_Intent))
			PANIC("intent out of range");
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Triggers_zero | (AsyncFuture_Triggers_one * triggers) |
		FIELD_SET(AsyncFuture_Intent, intent)
	;

	#if AsyncFuture_SAFE
		info = atom_exchg(&this->info, info, atom_sync);
		if (info)
			PANIC("future not zeroed");
	#else
		this->info = info;
	#endif

	this->task = task;
}



// macros to help construct AsyncError values
#define XERROR(N) ( \
	error_origin | FIELD(AsyncError_Class, AsyncFutureRes) | \
	FIELD_SET(AsyncError_Code, AsyncFutureRes_##N) \
)

#define XERROR_ALC(val) ( \
	error_origin | FIELD(AsyncError_Class, AlcRes) | \
	FIELD_SET(AsyncError_Code, (val)) \
)

#define XERROR_WAIT(val) ( \
	error_origin | FIELD(AsyncError_Class, AsyncWaitRes) | \
	FIELD_SET(AsyncError_Code, (val)) \
)

#define XERROR_UNKNOWN (error_origin | FIELD(AsyncError_Class, Unknown))



AsyncError AsyncFuture_invalidate(AsyncFuture *this) {
	constexpr auto error_origin = FIELD(AsyncError_Origin, FutureInvalidate);

	AsyncFutureInfo info = atom_or(&this->info,
		FLAGS(AsyncFuture, Resolved, Invalid),
		atom_sync
	);

	if (info & FLAG(AsyncFuture_Resolved)) {
		if (info & FLAG(AsyncFuture_Invalid))
			return XERROR(IsInvalid);

		if (info & FLAG(AsyncFuture_Error)) {
			#if AsyncFuture_SYNCERROR
				// wait for error lock
				while (info & AsyncFuture_ErrorLock) {
					CPU_YIELD;
					info = atom_get(&this->info, atom_acq);
				}
			#endif

			return XERROR(IsError);
		}

		return XERROR(IsResolved);
	}

	if (info & FIELD_MASK(AsyncFuture_Waiting)) {
		#if AsyncFuture_WAIT8
			auto const res = Async_wake8(&this->info, Async_wakeall);
		#else
			auto const res = Async_wake32(&this->info, Async_wakeall);
		#endif

		if (res) return XERROR_WAIT(res);
	}

	return AsyncError_none;
}



AsyncError AsyncFuture_geterror(AsyncFuture *this) {
	constexpr auto error_origin = FIELD(AsyncError_Origin, FutureGetError);

	AsyncFutureInfo info = atom_get(&this->info, atom_acq);

	// if future is not resolved
	if (!(info & FLAG(AsyncFuture_Resolved)))
		return AsyncError_none;

	if (info & FLAG(AsyncFuture_Invalid))
		return XERROR(IsInvalid);

	// if future doesnt have an error
	if (!(info & FLAG(AsyncFuture_Error)))
		return AsyncError_none;

	// wait for write lock
	if (info & AsyncFuture_ErrorLock) {
		do {
			CPU_YIELD;
			info = atom_get(&this->info, atom_acq);
		} while (info & AsyncFuture_ErrorLock);

		#if AsyncFuture_SAFE
			// check again for invalid flag
			if (info & FLAG(AsyncFuture_Invalid))
				return XERROR(IsInvalid);
		#endif
	}

	return this->error;
}



AsyncError AsyncFuture_seterror(
	AsyncFuture *this, AsyncError future_error
) {
	constexpr AsyncError error_origin = FIELD(AsyncError_Origin, FutureSetError);
	AsyncError error;

	// set error flag and lock future for writing error value
	AsyncFutureInfo info = atom_or(&this->info,
		FLAGS(AsyncFuture, Resolved, Error, ErrorLock),
		atom_sync
	);

	bool const was_resolved = info & FLAG(AsyncFuture_Resolved);

	if (was_resolved) {
		if (info & FLAG(AsyncFuture_Invalid))
			// if future is invalid we dont care about clearing the lock
			return XERROR(IsInvalid);

		// if an error was already set on the future
		if (info & FLAG(AsyncFuture_Error)) {
			if (info & AsyncFuture_ErrorLock) {
				#if AsyncFuture_SYNCERROR
					do {
						CPU_YIELD;
						info = atom_get(&this->info, atom_acq);
					} while (info & AsyncFuture_ErrorLock);

					#if AsyncFuture_SAFE || AsyncFuture_STRICT
						// check for invlid flag after waiting
						if (info & FLAG(AsyncFuture_Invalid))
							return XERROR(IsInvalid);
					#endif
				#endif
			} else {
				// if we set the lock, clear it
				info = atom_and(&this->info, ~AsyncFuture_ErrorLock, atom_sync);

				#if AsyncFuture_SAFE || AsyncFuture_STRICT
					if (info & FLAG(AsyncFuture_Invalid))
						return XERROR(IsInvalid);
				#endif
			}

			#if AsyncFuture_STRICT
				// invalidate the future if in strict mode
				error = XERROR(IsError);
				goto invalidate;
			#else
				return XERROR(IsError);
			#endif
		}

		// if the future was previously resolved successfully, we set the error
	}

	// we acquired a valid error lock because any previous seterror
	// call would have also set the error and resolved flag

	this->error = future_error;
	info = atom_and(&this->info, ~AsyncFuture_ErrorLock, atom_sync);

	// if future was already resolved we dont have to wake up threads
	if (was_resolved) {
		#if AsyncFuture_SAFE
			if (info & FLAG(AsyncFuture_Invalid))
				return XERROR(IsInvalid);
		#endif

		return AsyncError_none;
	}

	// if we were first to resolve the future
	// we have to handle waking threads
	if (info & FIELD_MASK(AsyncFuture_Waiting)) {
		#if AsyncFuture_WAIT8
			auto const res = Async_wake8(&this->info, Async_wakeall);
		#else
			auto const res = Async_wake32(&this->info, Async_wakeall);
		#endif

		if (info & FLAG(AsyncFuture_Invalid)) {
			if (res)
				return XERROR_UNKNOWN;

			return XERROR_WAIT(res);
		}

		// if waking threads failed invalidate future
		if (res) {
			error = XERROR_WAIT(res);
			goto invalidate;
		}
	}

	return AsyncError_none;

	[[maybe_unused]]
	invalidate:;
	info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
	return error;
}



AsyncError AsyncFuture_settask(
	AsyncFuture *this, AsyncTask task, AsyncIntent intent
) {
	constexpr AsyncError error_origin = FIELD(AsyncError_Origin, FutureSetTask);
	[[maybe_unused]]
	AsyncError error;

	#if AsyncFuture_SAFE
		if (intent > FIELD_MAX(AsyncFuture_Intent))
			PANIC("intent out of range");
	#endif

	// set write lock and intent
	AsyncFutureInfo info = atom_or(&this->info, AsyncFuture_TaskLock, atom_sync);

	// if future is already resolved
	if (info & FLAG(AsyncFuture_Resolved)) {
		// we dont need to remove lock if the future is already invalid
		if (info & FLAG(AsyncFuture_Invalid))
			return XERROR(IsInvalid);

		// same for error
		if (info & FLAG(AsyncFuture_Error)) {
			#if AsyncFuture_SYNCERROR
				if (info & AsyncFuture_ErrorLock) {
					do {
						CPU_YIELD;
						info = atom_get(&this->info, atom_acq);
					} while (info & AsyncFuture_ErrorLock);

					#if AsyncFuture_SAFE
						// check for invalid flag after waiting
						if (info & FLAG(AsyncFuture_Invalid))
							return XERROR(IsInvalid);
					#endif
				}
			#endif

			return XERROR(IsError);
		}

		// future is already successfully resolved we need to
		// remove lock if we set it
		if (!(info & AsyncFuture_TaskLock))
			info = atom_and(&this->info, ~AsyncFuture_TaskLock, atom_sync);

		// report to caller that feature is resolved
		return XERROR(IsResolved);
	}

	// if task lock is already set, another thread is setting
	// the task and we dont need to clear the lock
	if (info & AsyncFuture_TaskLock) {
		#if AsyncFuture_STRICT
			// invalidate future in strict mode and wake up waiting threads
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

			// if future was already resolved, do not wake threads
			if (!(info & FLAG(AsyncFuture_Resolved)))
				goto wake_isset;
		#endif

		return XERROR(IsSet);
	}

	// if task is already set clear lock
	if (info & FIELD_MASK(AsyncFuture_Intent)) {
		#if AsyncFuture_STRICT
			// invalidate task in strict mode
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
			bool const was_resolved = info & FLAGS(AsyncFuture_Resolved);
		#endif

		info = atom_and(&this->info, ~AsyncFuture_TaskLock, atom_sync);

		#if AsyncFuture_STRICT
			if (!was_resolved) wake_isset: {
				#if AsyncFuture_WAIT8
					auto const res = Async_wake8(&this->info, Async_wakeall);
				#else
					auto const res = Async_wake32(&this->info, Async_wakeall);
				#endif
				if (res) return XERROR_UNKNOWN;
			}
		#endif

		return XERROR(IsSet);
	}

	AsyncFutureInfo const intent_info = FIELD_SET(AsyncFuture_Intent, intent);

	// we have a valid write lock for the future
	// and can set the task and intent
	this->task = task;

	// clear lock and set intent
	// if cmpx succeeds on the first attempt,
	// we dont need to check again for invalidity
	if (
		atom_cmpx(&this->info, &info,
			(info & (~AsyncFuture_TaskLock)) | intent_info,
			atom_sync, atom_sync
		)
	) return AsyncError_none;

	while (
		!atom_cmpx(&this->info, &info,
			(info & (~AsyncFuture_TaskLock)) | intent_info,
			atom_sync, atom_sync
		)
	) {}

	if (info & FLAG(AsyncFuture_Resolved)) {
		if (info & FLAG(AsyncFuture_Invalid))
			return XERROR(IsInvalid);

		if (info & FLAG(AsyncFuture_Error)) {
			#if AsyncFuture_SYNCERROR
				if (info & AsyncFuture_ErrorLock) {
					do {
						CPU_YIELD;
						info = atom_get(&this->info, atom_acq);
					} while (info & AsyncFuture_ErrorLock);

					#if AsyncFuture_SAFE
						// check for invalid flag after waiting
						if (info & FLAG(AsyncFuture_Invalid))
							return XERROR(IsInvalid);
					#endif
				}
			#endif

			return XERROR(IsError);
		}
	}

	return AsyncError_none;
}



AsyncError AsyncFuture_wait(AsyncFuture *this, u64 timeout) {
	constexpr AsyncError error_origin = FIELD(AsyncError_Origin, FutureWait);
	AsyncError error;

	// set waiting flag
	AsyncFutureInfo info = atom_add(&this->info, AsyncFuture_Waiting_one, atom_sync);

	if (info & FLAG(AsyncFuture_Resolved)) {

		// if future was already resolved dont return an errror
		return AsyncError_none;
	}

	// update info to match our atomic add
	info += AsyncFuture_Waiting_one;
	do {
		#if AsyncFuture_WAIT8
			auto const res = Async_wait8(&this->info, (u8)info, timeout);
		#else
			auto const res = Async_wait32(&this->info, info, timeout);
		#endif

		switch (res) {
			default:
				error = XERROR_WAIT(res);
				goto wait_error;

			case AsyncWaitRes_Timeout:
				goto timeout;

			case AsyncWaitRes_Mismatch:
			case AsyncWaitRes_Ok:
		}

		info = atom_get(&this->info, atom_acq);
	} while (!(info & FLAG(AsyncFuture_Resolved)));

	info = atom_sub(&this->info, AsyncFuture_Waiting_one, atom_sync);

	resolved:;
	if (info & FLAG(AsyncFuture_Invalid))
		return XERROR(IsInvalid);

	if (info & FLAG(AsyncFuture_Error)) {
		if (info & AsyncFuture_ErrorLock) {
			do {
				CPU_YIELD;
				info = atom_get(&this->info, atom_acq);
			} while (info & AsyncFuture_ErrorLock);

			#if AsyncFuture_SAFE
				if (info & FLAG(AsyncFuture_Invalid))
					return XERROR(IsInvalid);
			#endif
		}

		return this->error;
	}

	return AsyncError_none;

	if (0) timeout: {
		info = atom_sub(&this->info, AsyncFuture_Waiting_one, atom_sync);
		#if AsyncFuture_SAFE
			if (info & FLAG(AsyncFuture_Resolved))
				goto resolved;
		#endif

		return XERROR(Timeout);
	}

	if (0) wait_error: {
		info = atom_sub(&this->info, AsyncFuture_Waiting_one, atom_sync);

		// if the future is already invalidated return error
		if (info & FLAGS(AsyncFuture, Resolved, Invalid))
			return error;

		// invalidate future
		info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

		if (info & FLAG(AsyncFuture_Resolved))
			return error;
		if (!(info & FLAG))


		if (info & FLAG(AsyncFuture_Resolved)) {
			if (info & FLAG(AsyncFuture_Invalid))
				return error;

			info = atom_or(&this->info, )

			goto invalidate;
		}

		goto invalidate_wake;
	}

	invalidate_wake:;
	info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
	#if AsyncFuture_WAIT8
		auto const res = Async_wake8(&this->info, Async_wakeall);
	#else
		auto const res = Async_wake32(&this->info, Async_wakeall);
	#endif
	if (res) return XERROR_UNKNOWN;
	return error;
}



AsyncIntent AsyncFuture_resolve(AsyncFuture *this, AsyncResult *result) {
	constexpr auto error_origin = FIELD(AsyncError_Origin, FutureResolve);
	AsyncError error;

	AsyncFutureInfo info = atom_sub(&this->info, AsyncFuture_Triggers_one, atom_sync);
	if (info & FLAG(AsyncFuture_Resolved))
		goto already_resolved;

	auto const count = info & AsyncFuture_Triggers_mask;

	// if count is not 1 return
	if (count > AsyncFuture_Triggers_single)
		return AsyncIntent_Yield;

	// check if count underflowed
	if (count < AsyncFuture_Triggers_single) {
		error = XERROR(ErrUnderflow);
		goto invalidate_wake;
	}

	// resolve future
	info = atom_or(&this->info, FLAG(AsyncFuture_Resolved), atom_sync);

	#if AsyncFuture_SAFE
		// check for error/invalidation again
		if (info & FLAG(AsyncFuture_Resolved))
			goto already_resolved;
	#endif

	// future is resolved,
	AsyncIntent const intent = FIELD_GET(AsyncFuture_Intent, info);
	// if intent for task is set, return the task
	if (intent) {
		// wait for write lock
		if (info & FLAG(AsyncFuture_Lock)) {
			do {
				CPU_YIELD;
				info = atom_get(&this->info, atom_acq);
			} while (info & FLAG(AsyncFuture_Lock));

			#if AsyncFuture_SAFE
				// if future got invalidated while waiting
				if (info & FLAG(AsyncFuture_Invalid)) {
					error = XERROR(IsInvalid);

					// since this thread resolved the future first
					// we need to wake up waiting threads even though
					// another thread invalidated the future
					if (info & FLAG(AsyncFuture_Waiting)) {
						#if AsyncFuture_WAIT8
							auto const res = Async_wake8(&this->info, Async_wakeall);
						#else
							auto const res = Async_wake32(&this->info, Async_wakeall);
						#endif
						if (res) error = XERROR_UNKNOWN;
					}

					goto return_error;
				}

			#endif
		}

		result->task = this->task;
	}

	// wake up threads waiting on the state
	if (info & FLAG(AsyncFuture_Waiting)) {
		#if AsyncFuture_WAIT8
			auto const res = Async_wake8(&this->info, Async_wakeall);
		#else
			auto const res = Async_wake32(&this->info, Async_wakeall);
		#endif

		// invalidate future if waking failed
		if (res) {
			error = XERROR_WAIT(res);
			goto invalidate;
		}
	}

	return intent;

	if (0) already_resolved: {
		if (info & FLAG(AsyncFuture_Invalid)) {
			error = XERROR(IsInvalid);
			goto return_error;
		}

		if (info & FLAG(AsyncFuture_Error))
			error = XERROR(IsError);
		else
			error = XERROR(IsResolved);

		goto invalidate;
	}

	if (0) invalidate: {
		info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
	}

	if (0) invalidate_wake: {
		info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

		// if only the waiting flag is set and not resolved flag
		if ((info & FLAGS(AsyncFuture, Resolved, Waiting)) == FLAG(AsyncFuture_Waiting)) {
			#if AsyncFuture_WAIT8
				auto const res = Async_wake8(&this->info, Async_wakeall);
			#else
				auto const res = Async_wake32(&this->info, Async_wakeall);
			#endif
			// if wake failed, set error to unknown because
			// we cant return multiple errors
			if (res) error = XERROR_UNKNOWN;
		}
	}

	return_error:;
	result->error = error;
	return AsyncIntent_Error;
}



#undef XERROR
#undef XERROR_WAIT
#undef XERROR_ALC
#undef XERROR_UNKNOWN
