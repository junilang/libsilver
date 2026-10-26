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
	AsyncRes error;
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



// macros to help construct AsyncRes values
#define XRES(N) ( \
	error_origin | FIELD(AsyncRes_Class, AsyncFutureRes) | \
	FIELD_SET(AsyncRes_Code, AsyncFutureRes_##N) \
)

#define XRES_ALC(val) ( \
	error_origin | FIELD(AsyncRes_Class, AlcRes) | \
	FIELD_SET(AsyncRes_Code, (val)) \
)

#define XRES_WAIT(val) ( \
	error_origin | FIELD(AsyncRes_Class, AsyncWaitRes) | \
	FIELD_SET(AsyncRes_Code, (val)) \
)

#define XRES_UNKNOWN (error_origin | FIELD(AsyncRes_Class, Unknown))



AsyncRes AsyncFuture_invalidate(AsyncFuture *this) {
	constexpr auto error_origin = FIELD(AsyncRes_Origin, FutureInvalidate);

	AsyncFutureInfo info = atom_or(&this->info,
		FLAGS(AsyncFuture, Resolved, Invalid),
		atom_sync
	);

	if (info & FLAG(AsyncFuture_Resolved)) {
		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

		if (info & FLAG(AsyncFuture_Error)) {
			#if AsyncFuture_SYNCERROR
				// wait for error lock
				while (info & AsyncFuture_ErrorLock) {
					CPU_YIELD;
					info = atom_get(&this->info, atom_acq);
				}
			#endif

			return XRES(IsError);
		}

		return XRES(IsResolved);
	}

	if (info & FIELD_MASK(AsyncFuture_Waiting)) {
		#if AsyncFuture_WAIT8
			auto const res = Async_wake8(&this->info, Async_wakeall);
		#else
			auto const res = Async_wake32(&this->info, Async_wakeall);
		#endif

		if (res) return XRES_WAIT(res);
	}

	return AsyncRes_Ok;
}



AsyncRes AsyncFuture_geterror(AsyncFuture *this) {
	constexpr auto error_origin = FIELD(AsyncRes_Origin, FutureGetError);

	AsyncFutureInfo info = atom_get(&this->info, atom_acq);

	// if future is not resolved
	if (!(info & FLAG(AsyncFuture_Resolved)))
		return AsyncRes_Ok;

	if (info & FLAG(AsyncFuture_Invalid))
		return XRES(IsInvalid);

	// if future doesnt have an error
	if (!(info & FLAG(AsyncFuture_Error)))
		return AsyncRes_Ok;

	// wait for write lock
	if (info & AsyncFuture_ErrorLock) {
		do {
			CPU_YIELD;
			info = atom_get(&this->info, atom_acq);
		} while (info & AsyncFuture_ErrorLock);

		#if AsyncFuture_SAFE
			// check again for invalid flag
			if (info & FLAG(AsyncFuture_Invalid))
				return XRES(IsInvalid);
		#endif
	}

	return this->error;
}



AsyncRes AsyncFuture_seterror(
	AsyncFuture *this, AsyncRes future_error
) {
	constexpr AsyncRes error_origin = FIELD(AsyncRes_Origin, FutureSetError);
	AsyncRes error;

	// set error flag and lock future for writing error value
	AsyncFutureInfo info = atom_or(&this->info,
		FLAGS(AsyncFuture, Resolved, Error, ErrorLock),
		atom_sync
	);

	bool const was_resolved = info & FLAG(AsyncFuture_Resolved);

	if (was_resolved) {
		if (info & FLAG(AsyncFuture_Invalid))
			// if future is invalid we dont care about clearing the lock
			return XRES(IsInvalid);

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
							return XRES(IsInvalid);
					#endif
				#endif
			} else {
				// if we set the lock, clear it
				info = atom_and(&this->info, ~AsyncFuture_ErrorLock, atom_sync);

				#if AsyncFuture_SAFE || AsyncFuture_STRICT
					if (info & FLAG(AsyncFuture_Invalid))
						return XRES(IsInvalid);
				#endif
			}

			#if AsyncFuture_STRICT
				// invalidate the future if in strict mode
				error = XRES(IsError);
				goto invalidate;
			#else
				return XRES(IsError);
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
				return XRES(IsInvalid);
		#endif

		return AsyncRes_Ok;
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
				return XRES_UNKNOWN;

			return XRES_WAIT(res);
		}

		// if waking threads failed invalidate future
		if (res) {
			error = XRES_WAIT(res);
			goto invalidate;
		}
	}

	return AsyncRes_Ok;

	[[maybe_unused]]
	invalidate:;
	info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
	return error;
}

AsyncRes AsyncFuture_settask(
	AsyncFuture *this, AsyncTask task, AsyncIntent intent
) {
	constexpr AsyncRes error_origin = FIELD(AsyncRes_Origin, FutureSetTask);
	[[maybe_unused]]
	AsyncRes error;

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
			return XRES(IsInvalid);

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
							return XRES(IsInvalid);
					#endif
				}
			#endif

			return XRES(IsError);
		}

		// future is already successfully resolved we need to
		// remove lock if we set it
		if (!(info & AsyncFuture_TaskLock))
			info = atom_and(&this->info, ~AsyncFuture_TaskLock, atom_sync);

		// report to caller that feature is resolved
		return XRES(IsResolved);
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

		return XRES(IsSet);
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
				if (res) return XRES_UNKNOWN;
			}
		#endif

		return XRES(IsSet);
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
			atom_sync, atom_acq
		)
	) return AsyncRes_Ok;

	while (
		!atom_cmpx(&this->info, &info,
			(info & (~AsyncFuture_TaskLock)) | intent_info,
			atom_sync, atom_acq
		)
	) {}

	if (info & FLAG(AsyncFuture_Resolved)) {
		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

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
							return XRES(IsInvalid);
					#endif
				}
			#endif

			return XRES(IsError);
		}
	}

	return AsyncRes_Ok;
}



AsyncRes AsyncFuture_wait(AsyncFuture *this, u64 timeout) {
	constexpr AsyncRes error_origin = FIELD(AsyncRes_Origin, FutureWait);
	AsyncRes error;

	// set waiting flag
	AsyncFutureInfo info = atom_add(&this->info, AsyncFuture_Waiting_one, atom_sync);

	if (info & FLAG(AsyncFuture_Resolved)) {

		// if future was already resolved dont return an errror
		return AsyncRes_Ok;
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
				error = XRES_WAIT(res);
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
		return XRES(IsInvalid);

	if (info & FLAG(AsyncFuture_Error)) {
		if (info & AsyncFuture_ErrorLock) {
			do {
				CPU_YIELD;
				info = atom_get(&this->info, atom_acq);
			} while (info & AsyncFuture_ErrorLock);

			#if AsyncFuture_SAFE
				if (info & FLAG(AsyncFuture_Invalid))
					return XRES(IsInvalid);
			#endif
		}

		return this->error;
	}

	return AsyncRes_Ok;

	if (0) timeout: {
		info = atom_sub(&this->info, AsyncFuture_Waiting_one, atom_sync);
		#if AsyncFuture_SAFE
			if (info & FLAG(AsyncFuture_Resolved))
				goto resolved;
		#endif

		return XRES(Timeout);
	}

	if (0) wait_error: {
		info = atom_sub(&this->info, AsyncFuture_Waiting_one, atom_sync);

		// if the future is already invalidated return error
		if (info & FLAGS(AsyncFuture, Resolved, Invalid))
			return error;

		// invalidate future
		info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

		// wake up waiting threads if future wasnt resolved
		if (
			(!(info & FLAG(AsyncFuture_Resolved))) &&
			(info & FIELD_MASK(AsyncFuture_Waiting))
		) {
			#if AsyncFuture_WAIT8
				auto const res = Async_wake8(&this->info, Async_wakeall);
			#else
				auto const res = Async_wake32(&this->info, Async_wakeall);
			#endif
			if (res) return XRES_UNKNOWN;
		}

		return error;
	}
}



AsyncRes AsyncFuture_resolve(AsyncFuture *this, AsyncTask *out_task) {
	constexpr auto error_origin = FIELD(AsyncRes_Origin, FutureResolve);
	//AsyncRes error;

	AsyncFutureInfo info = atom_sub(&this->info, AsyncFuture_Triggers_one, atom_sync);

	if (info & FLAG(AsyncFuture_Resolved))
		goto already_resolved;

	auto const count = info & FIELD_MASK(AsyncFuture_Triggers);

	// if count is more than one return
	if (count > AsyncFuture_Triggers_single) {
		return AsyncIntent_Yield;
	}

	// if underflow occured
	if (count < AsyncFuture_Triggers_single) {
		#if AsyncFuture_STRICT
			// invalidate future in strict mode
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

			if (!(info & FLAG(AsyncFuture_Resolved))) {
				// wake threads if not resolved
				#if AsyncFuture_WAIT8
					auto const res = Async_wake8(&this->info, Async_wakeall);
				#else
					auto const res = Async_wake32(&this->info, Async_wakeall);
				#endif
				if (res) return XRES_UNKNOWN;
			}
		#endif

		return XRES(ErrUnderflow);
	}

	// resolve future
	info = atom_or(&this->info, FLAG(AsyncFuture_Resolved), atom_sync);

	if (info & FLAG(AsyncFuture_Resolved)) already_resolved: {
		if (info & FLAG(AsyncFuture_Invalid)) {
			return XRES(IsInvalid);
		}

		if (info & FLAG(AsyncFuture_Error)) {
			#if AsyncFuture_SYNCERROR
				if (info & AsyncFuture_ErrorLock) {
					do {
						CPU_YIELD;
						info = atom_get(&this->info, atom_acq);
					} while (info & AsyncFuture_ErrorLock);

					#if AsyncFuture_SAFE
						if (info & FLAG(AsyncFuture_Invalid))
							return XRES(IsInvalid);
					#endif
				}

			#endif
		}

		#if AsyncFuture_STRICT
			// invalidate future in strict mode if future was already resolved
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
			// we dont need to wake threads because the future is already resolved

			#if AsyncFuture_SAFE
				// another thread invalidated first
				if (info & FLAG(AsyncFuture_Invalid))
					return XRES(IsInvalid);
			#endif
		#endif

		return XRES(IsResolved);
	}

	// wait for task lock
	if (info & AsyncFuture_TaskLock) {
		do {
			CPU_YIELD;
			info = atom_get(&this->info, atom_acq);
		} while (info & AsyncFuture_TaskLock);

		#if AsyncFuture_SAFE
			if (info & FLAG(AsyncFuture_Invalid))
				return XRES(IsInvalid);

			if (info & FLAG(AsyncFuture_Error)) {
				#if AsyncFuture_SYNCERROR
					if (info & AsyncFuture_ErrorLock) {
						do {
							CPU_YIELD;
							info = atom_get(&this->info, atom_acq);
						} while (info & AsyncFuture_ErrorLock);

						#if AsyncFuture_SAFE
							if (info & FLAG(AsyncFuture_Invalid))
								return XRES(IsInvalid);
						#endif
					}
				#endif

				return XRES(IsError);
			}
		#endif
	}

	AsyncIntent intent = FIELD_GET(AsyncFuture_Intent, info);
	if (intent)
		*out_task = this->task;

	// wake up threads
	if (info & FIELD_MASK(AsyncFuture_Waiting)) {
		#if AsyncFuture_WAIT8
			auto const res = Async_wake8(&this->info, Async_wakeall);
		#else
			auto const res = Async_wake32(&this->info, Async_wakeall);
		#endif

		// invalide future
		if (res) {
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
			return XRES_WAIT(res);
		}

	}

	return intent;
}



#undef XRES
#undef XRES_WAIT
#undef XRES_ALC
#undef XRES_UNKNOWN
