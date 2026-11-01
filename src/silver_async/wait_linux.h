struct AsyncTime {
	linux_timespec ts;
};


AsyncWaitRes Async_timeout(u64 duration, AsyncTime *out) {
	linux_timespec ts;
	auto res = linux_clock_gettime_fast(CLOCK_MONOTONIC, &ts);
	if (res < 0)
		return AsyncWaitRes_ErrClock;

	ts.tv_sec += (typeof(ts.tv_sec))(duration / Async_seconds);
	ts.tv_nsec += (typeof(ts.tv_sec))(duration % Async_seconds);
	out->ts = ts;

	return AsyncWaitRes_Ok;
}

AsyncWaitRes Async_ZZwait(Ptr addr, usize val, const AsyncTime *timeout, uword flags) {
	const linux_timespec *tp = timeout ? &timeout->ts : nullptr;

	auto res = linux_futex_wait(
		addr, val, FUTEX_BITSET_MATCH_ANY,
		flags | FUTEX2_PRIVATE,
		tp, CLOCK_MONOTONIC
	);

	if (res < 0) switch (-res) {
		case EAGAIN: // expected value did not match
			return AsyncWaitRes_Mismatch;
		case EINTR: // interrupted by signal
			return AsyncWaitRes_Interrupt;
		case ETIMEDOUT: // timed out
			return AsyncWaitRes_Timeout;
		case EFAULT:
		case EINVAL: // invalid timeout input
			return AsyncWaitRes_ErrInvalid;

		default:
			return AsyncWaitRes_ErrUnknown;
	}

	return AsyncWaitRes_Ok;
}

AsyncWaitRes Async_sleep(u64 duration) {
	if (duration == 0) return AsyncWaitRes_Ok;

	const linux_timespec ts = {
		.tv_sec = (typeof(ts.tv_nsec))(duration / Async_seconds),
		.tv_nsec = (typeof(ts.tv_nsec))(duration % Async_seconds)
	};

	auto res = linux_clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
	if (res < 0) switch (-res) {
		default:
			return AsyncWaitRes_ErrUnknown;
	}

	return AsyncWaitRes_Ok;
}

AsyncWaitRes Async_wait8(Ptr addr, u8 val, const AsyncTime *timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U8);
}

AsyncWaitRes Async_wait16(Ptr addr, u16 val, const AsyncTime *timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U16);
}

AsyncWaitRes Async_wait32(Ptr addr, u32 val, const AsyncTime *timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U32);
}

AsyncWaitRes Async_wait64(Ptr addr, u64 val, const AsyncTime *timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U64);
}

constexpr int Async_wakeall = int_max;
constexpr int Async_wakeone = 1;


AsyncWaitRes Async_ZZwake(Ptr addr, int *io_n, uword flags) {
	auto res = linux_futex_wake(
		addr, FUTEX_BITSET_MATCH_ANY, *io_n,
		flags | FUTEX2_PRIVATE
	);

	if (res < 0) switch (-res) {
		default:
			return AsyncWaitRes_ErrUnknown;
	}

	*io_n = (int)res;
	return AsyncWaitRes_Ok;
}

AsyncWaitRes Async_wake8_get(Ptr addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U8);
}

AsyncWaitRes Async_wake16_get(Ptr addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U16);
}

AsyncWaitRes Async_wake32_get(Ptr addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U32);
}

AsyncWaitRes Async_wake64_get(Ptr addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U64);
}

AsyncWaitRes Async_wake8(Ptr addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U8);
}

AsyncWaitRes Async_wake16(Ptr addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U16);
}

AsyncWaitRes Async_wake32(Ptr addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U32);
}

AsyncWaitRes Async_wake64(Ptr addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U64);
}
