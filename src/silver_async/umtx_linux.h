typedef enum : u32 {
	umtxResult_OK = 0,
	umtxResult_MISMATCH,
	umtxResult_TIMEOUT,
	umtxResult_INTERRUPT,
	umtxResult_INVALID,
	umtxResult_ERROR
} umtxResult;

typedef u32 umtx;

typedef struct timespec umtxTimeout;

#define umtx_SECONDS 1'000'000'000ull
#define umtx_MILLIS 1'000'000ull
#define umtx_MICROS 1'000ull
#define umtx_NANOS 1ull

umtxTimeout umtx_timeout(u64 duration) {
	umtxTimeout ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	ts.tv_sec += (typeof(ts.tv_sec))(duration / umtx_SECONDS);
	ts.tv_nsec += (typeof(ts.tv_nsec))(duration % umtx_SECONDS);

	return ts;
}

umtxResult umtx_wait_timeout(
	_Atomic umtx *this, umtx expected_value, const umtxTimeout *timeout
) {
	long result = syscall(
		SYS_futex, this, FUTEX_WAIT_BITSET_PRIVATE, expected_value,
		timeout, nullptr, FUTEX_BITSET_MATCH_ANY
	);

	if (result == 0) return umtxResult_OK;

	if (result == -1)
	{
		switch (errno) {
			case EAGAIN: // expected value did not match
				return umtxResult_MISMATCH;

			case EINTR: // interrupted by signal
				return umtxResult_INTERRUPT;

			case EFAULT:
			case EINVAL: // invalid timeout input
				return umtxResult_INVALID;

			case ETIMEDOUT: // timed out
				return umtxResult_TIMEOUT;
		}
	}

	UNREACHABLE;
}

umtxResult umtx_wait(_Atomic umtx *this, umtx expected_value) {
	return umtx_wait_timeout(this, expected_value, nullptr);
}

#define umtx_WAKE_ALL INT_MAX
#define umtx_WAKE_ONE 1

umtxResult umtx_wake_get(_Atomic umtx *this, u32 *io_n) {
	long result = syscall(
		SYS_futex, this, FUTEX_WAKE_BITSET_PRIVATE, *io_n,
		nullptr, nullptr, FUTEX_BITSET_MATCH_ANY
	);

	if (result == -1)
		return umtxResult_ERROR;

	*io_n = (u32)result;
	return umtxResult_OK;
}

umtxResult umtx_wake(_Atomic umtx *this, u32 n) {
	long result = syscall(
		SYS_futex, this, FUTEX_WAKE_BITSET_PRIVATE, n,
		nullptr, nullptr, FUTEX_BITSET_MATCH_ANY
	);

	if (result == -1)
		return umtxResult_ERROR;

	return umtxResult_OK;
}

umtxResult umtx_wake_all(_Atomic umtx *this) {
	return umtx_wake(this, umtx_WAKE_ALL);
}

umtxResult umtx_wake_one(_Atomic umtx *this) {
	return umtx_wake(this, umtx_WAKE_ONE);
}
