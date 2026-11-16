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

ENUM_GENERATE(AsyncFutureRes, u8,
	Ok,
	Timeout,
	IsSet,
	IsResolved,
	IsError,
	IsInvalid,
	ErrUnderflow,
	ErrUnknown
)

typedef u32 AsyncFutureInfo; enum {
	FLAGS_DEF(AsyncFuture,
		// future is resolved
		Resolved,
		// future is resolved with error
		Error,
		// an error occured during interaction with future (error is not set)
		Invalid,
		// lock set when thread is writing an error
		ErrorLock,
		// lock set when thread is setting the task
		TaskLock
	),

	// task intent, 2 bits is enough for
	// 00=no task (Yield), 01=Resume, 10=Suspend, 11=Call
	FIELD_DEF(AsyncFuture_Intent, 2),

	// number of threads waiting on the future
	FIELD_DEF_FROM(AsyncFuture_Waiting, 8, 10),

	// number of resolve calls needed to complete the future
	FIELD_DEF_UNTIL(AsyncFuture_Triggers, 32),
	AsyncFutureInfo_width
};

#if AsyncFuture_WAIT8
	// ensure that the state header fits into a single byte
	static_assert(AsyncFuture_Intent_FIELD_END < 8);
#endif

static_assert(AsyncFuture_Intent_FIELD_END < AsyncFuture_Waiting_FIELD);
static_assert(AsyncFutureInfo_width <= 32);



AsyncFutureInfo constexpr AsyncFuture_ErrorLock = FLAG(AsyncFuture_ErrorLock);
AsyncFutureInfo constexpr AsyncFuture_TaskLock = FLAG(AsyncFuture_TaskLock);

AsyncFutureInfo constexpr AsyncFuture_Waiting_one = FIELD_SET(AsyncFuture_Waiting, 1);

AsyncFutureInfo constexpr AsyncFuture_Triggers_one = FIELD_SET(AsyncFuture_Triggers, 1);

// number of top bits in triggers field used for underflow detection
auto constexpr AsyncFuture_Triggers_underflowbits = 2;

// the "zero" value has the top underflow bits set to ones
AsyncFutureInfo constexpr AsyncFuture_Triggers_zero =
	FIELD_MASK_UPPER(AsyncFuture_Triggers, AsyncFuture_Triggers_underflowbits)
;

// the treshold for detecting succesfull resolves and underflows
AsyncFutureInfo constexpr AsyncFuture_Triggers_single =
	AsyncFuture_Triggers_zero | AsyncFuture_Triggers_one
;

u32 constexpr AsyncFuture_maxtriggers =
	FIELD_MAX(AsyncFuture_Triggers) >> AsyncFuture_Triggers_underflowbits
;

typedef struct {
	_Atomic u32_lf info;
	AsyncRes error;
	AsyncTask task;
} AsyncFuture;



void AsyncFuture_init(AsyncFuture *this, u32 triggers) {
	#if AsyncFuture_SAFE
		ASSERT(triggers <= AsyncFuture_maxtriggers);
		ASSERT(triggers != 0);
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Triggers_zero | (AsyncFuture_Triggers_one * triggers)
	;

	#if AsyncFuture_SAFE
		ASSERT(
			atom_exchg(&this->info, info, atom_sync) == 0
			, "future not zeroed"
		);
	#else
		this->info = info;
	#endif
}



void AsyncFuture_inittask(
	AsyncFuture *this, u32 triggers, AsyncTask task, AsyncIntent intent
) {
	#if AsyncFuture_SAFE
		ASSERT(triggers <= AsyncFuture_maxtriggers);
		ASSERT(triggers != 0);
		ASSERT(intent <= FIELD_MAX(AsyncFuture_Intent));
	#endif

	AsyncFutureInfo info =
		AsyncFuture_Triggers_zero | (AsyncFuture_Triggers_one * triggers) |
		FIELD_SET(AsyncFuture_Intent, intent)
	;

	#if AsyncFuture_SAFE
		ASSERT(
			atom_exchg(&this->info, info, atom_sync) == 0
			, "future not zeroed"
		);
	#else
		this->info = info;
	#endif

	this->task = task;
}



// macros to help construct AsyncRes values
#define XRES(N) ( \
	res_origin | FIELD(AsyncRes_Class, AsyncFutureRes) | \
	FIELD_SET(AsyncRes_Code, AsyncFutureRes_##N) \
)

#define XRES_ALC(val) ( \
	res_origin | FIELD(AsyncRes_Class, AlcRes) | \
	FIELD_SET(AsyncRes_Code, (val)) \
)

#define XRES_WAIT(val) ( \
	res_origin | FIELD(AsyncRes_Class, AsyncWaitRes) | \
	FIELD_SET(AsyncRes_Code, (val)) \
)

#define XRES_UNKNOWN (res_origin | FIELD(AsyncRes_Class, Unknown))

#if AsyncFuture_WAIT8
	#define XWAKE(infop) Async_wake8((infop), Async_wakeall)
#else
	#define XWAKE(infop) Async_wake32((infop), Async_wakeall)
#endif

#if AsyncFuture_SAFE
	#define XSYNCERROR_SAFE { \
		if (info & FLAG(AsyncFuture_Invalid)) \
			return XRES(IsInvalid); \
	}
#else
	#define XSYNCERROR_SAFE
#endif

#if AsyncFuture_SYNCERROR
	#define XSYNCERROR { \
		if (info & AsyncFuture_ErrorLock) { \
			do { \
				CPU_YIELD; \
				info = atom_get(&this->info, atom_acq); \
			} while (info & AsyncFuture_ErrorLock); \
			XSYNCERROR_SAFE \
		} \
		return XRES(IsError); \
	}
#else
	#define XSYNCERROR { return XRES(IsError); }
#endif


AsyncRes AsyncFuture_invalidate(AsyncFuture *this) {
	constexpr auto res_origin = FIELD(AsyncRes_Origin, FutureInvalidate);

	AsyncFutureInfo info = atom_or(&this->info,
		FLAGS(AsyncFuture, Resolved, Invalid),
		atom_sync
	);

	if (info & FLAG(AsyncFuture_Resolved)) {
		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

		if (info & FLAG(AsyncFuture_Error))
			XSYNCERROR;

		return XRES(IsResolved);
	}

	if (info & FIELD_MASK(AsyncFuture_Waiting)) {
		auto const res = XWAKE(&this->info);
		if (res) return XRES_WAIT(res);
	}

	return AsyncRes_Ok;
}



AsyncRes AsyncFuture_geterror(AsyncFuture *this) {
	constexpr auto res_origin = FIELD(AsyncRes_Origin, FutureGetError);

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



AsyncRes AsyncFuture_seterror(AsyncFuture *this, AsyncRes future_error) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, FutureSetError);
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

	// set error and clear lock
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
		auto const res = XWAKE(&this->info);

		// we always check for invalidity because we have
		// to invalidate the future on wake failure
		if (info & FLAG(AsyncFuture_Invalid)) {
			if (res) return XRES_UNKNOWN;
			return XRES(IsInvalid);
		}

		// if waking threads failed invalidate future
		if (res) {
			error = XRES_WAIT(res);
			goto invalidate;
		}
	}

	return AsyncRes_Ok;

	invalidate:;
	info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);
	return error;
}

AsyncRes AsyncFuture_settask(
	AsyncFuture *this, AsyncTask task, AsyncIntent intent
) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, FutureSetTask);
	[[maybe_unused]]
	AsyncRes error;

	#if AsyncFuture_SAFE
		ASSERT(
			intent <= FIELD_MAX(AsyncFuture_Intent)
			, "invalid intent for task"
		);
	#endif

	// set task write lock
	AsyncFutureInfo info = atom_or(&this->info, AsyncFuture_TaskLock, atom_sync);

	// if future is already resolved
	if (info & FLAG(AsyncFuture_Resolved)) {
		// we dont need to remove lock if the future is already invalid
		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

		// same for error
		if (info & FLAG(AsyncFuture_Error))
			XSYNCERROR;

		// future is already successfully resolved we need to
		// remove lock if we set it
		if (!(info & AsyncFuture_TaskLock)) {
			info = atom_and(&this->info, ~AsyncFuture_TaskLock, atom_sync);
			#if AsyncFuture_SAFE
				if (info & FLAG(AsyncFuture_Invalid))
					return XRES(IsInvalid);
			#endif
		}

		// report to caller that future is resolved
		return XRES(IsResolved);
	}

	// if task lock is already set, another thread is setting
	// the task and we dont need to clear the lock
	if (info & AsyncFuture_TaskLock) {
		#if AsyncFuture_STRICT
			// invalidate future in strict mode
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

			// if we were the first to resolve the future, we need to wake waiting threads
			if (!(info & FLAG(AsyncFuture_Resolved))) {
				if (info & FIELD_MASK(AsyncFuture_Waiting)) {
					auto const res = XWAKE(&this->info);
					if (res) return XRES_UNKNOWN;
				}
				return XRES(IsSet);
			}

			if (info & FLAG(AsyncFuture_Invalid))
				return XRES(IsInvalid);

			if (info & FLAG(AsyncFuture_Error))
				XSYNCERROR;

			return XRES(IsResolved);
		#endif

		return XRES(IsSet);
	}

	// if task is already set clear lock
	if (info & FIELD_MASK(AsyncFuture_Intent)) {
		#if AsyncFuture_STRICT
			// invalidate task in strict mode
			info = atom_or(
				&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync
			);

			// clear lock
			atom_and(&this->info, ~AsyncFuture_TaskLock, atom_sync);

			if (!(info & FLAG(AsyncFuture_Resolved))) {
				if (info & FIELD_MASK(AsyncFuture_Waiting)) {
					auto const res = XWAKE(&this->info);
					if (res) return XRES_UNKNOWN;
				}
				return XRES(IsSet);
			}

		#else
			info = atom_and(&this->info, ~AsyncFuture_TaskLock, atom_sync);

			if (!(info & FLAG(AsyncFuture_Resolved)))
				return XRES(IsSet);
		#endif

		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

		if (info & FLAG(AsyncFuture_Error))
			XSYNCERROR;

		return XRES(IsResolved);
	}

	// we have a valid write lock for the future
	// and can set the task and intent
	this->task = task;

	// set task lock flag on our version of info
	// so that the cmpx can succeed
	info |= AsyncFuture_TaskLock;
	AsyncFutureInfo const intent_info = FIELD_SET(AsyncFuture_Intent, intent);


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

		if (info & FLAG(AsyncFuture_Error))
			XSYNCERROR;

		return XRES(IsResolved);
	}

	return AsyncRes_Ok;
}



AsyncRes AsyncFuture_wait(AsyncFuture *this, const AsyncTime *timeout) {
	constexpr AsyncRes res_origin = FIELD(AsyncRes_Origin, FutureWait);
	AsyncRes error;

	AsyncFutureInfo info = atom_add(&this->info, AsyncFuture_Waiting_one, atom_sync);

	if (info & FLAG(AsyncFuture_Resolved)) {
		#if AsyncFuture_SAFE
			// this is unnecessary because other threads wont
			// attempt to wake waiters if they werent the first
			// one to resolve the future
			atom_sub(&this->info, AsyncFuture_Waiting_one, atom_sync);
		#endif

		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

		if (info & FLAG(AsyncFuture_Error))
			XSYNCERROR;

		// if future was already resolved dont return an errror
		return AsyncRes_Ok;
	}

	// update info to match the atomic add
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

	if (info & FLAG(AsyncFuture_Error))
		XSYNCERROR;

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
			return XRES_UNKNOWN;

		// invalidate future
		info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

		// wake up waiting threads if future wasnt resolved
		// and has waiters
		if (
			(!(info & FLAG(AsyncFuture_Resolved))) &&
			(info & FIELD_MASK(AsyncFuture_Waiting))
		) {
			auto const res = XWAKE(&this->info);
			if (res) return XRES_UNKNOWN;
		}

		return error;
	}

	__builtin_unreachable();
}



AsyncRes AsyncFuture_resolve(AsyncFuture *this, AsyncTask *out_task) {
	constexpr auto res_origin = FIELD(AsyncRes_Origin, FutureResolve);

	AsyncFutureInfo info = atom_sub(&this->info, AsyncFuture_Triggers_one, atom_sync);

	if (info & FLAG(AsyncFuture_Resolved))
		goto already_resolved;

	auto const count = info & FIELD_MASK(AsyncFuture_Triggers);

	// if count is more than one return
	if (count > AsyncFuture_Triggers_single)
		return AsyncIntent_Yield;

	// if underflow occured
	if (count < AsyncFuture_Triggers_single) {
		#if AsyncFuture_STRICT
			// invalidate future in strict mode
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

			// wake up waiters
			if (
				(!(info & FLAG(AsyncFuture_Resolved))) &&
				(info & FIELD_MASK(AsyncFuture_Waiting))
			) {
				auto const res = XWAKE(&this->info);
				if (res) return XRES_UNKNOWN;
			}
		#endif

		return XRES(ErrUnderflow);
	}

	// resolve future
	info = atom_or(&this->info, FLAG(AsyncFuture_Resolved), atom_sync);

	if (info & FLAG(AsyncFuture_Resolved)) already_resolved: {
		if (info & FLAG(AsyncFuture_Invalid))
			return XRES(IsInvalid);

		if (info & FLAG(AsyncFuture_Error))
			XSYNCERROR;

		#if AsyncFuture_STRICT
			// invalidate future in strict mode if future was already resolved
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

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

			if (info & FLAG(AsyncFuture_Error))
				XSYNCERROR;
		#endif
	}

	AsyncIntent const intent = FIELD_GET(AsyncFuture_Intent, info);
	if (intent)
		*out_task = this->task;

	// wake up threads
	if (info & FIELD_MASK(AsyncFuture_Waiting)) {
		auto const res = XWAKE(&this->info);

		// invalide future if waking threads failed
		if (res) {
			info = atom_or(&this->info, FLAGS(AsyncFuture, Resolved, Invalid), atom_sync);

			#if AsyncFuture_SAFE
				if (info & FLAG(AsyncFuture_Invalid))
					return XRES_UNKNOWN;
			#endif

			return XRES_WAIT(res);
		}
	}

	return intent;
}



#undef XRES
#undef XRES_WAIT
#undef XRES_ALC
#undef XRES_UNKNOWN
#undef XWAKE
#undef XSYNCERROR
#undef XSYNCERROR_SAFE
