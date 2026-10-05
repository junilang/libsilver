AsyncWaitRes Async_ZZwait(Ptr addr, usize val, u64 timeout, uword flags) {
	linux_timespec ts;
	const linux_timespec *tp = nullptr;
	if (timeout) {
		auto res = linux_clock_gettime(CLOCK_MONOTONIC, &ts);
		if (res < 0)
			return AsyncWaitRes_ErrClock;

		ts.tv_sec += (typeof(ts.tv_sec))(timeout / Async_seconds);
		ts.tv_nsec += (typeof(ts.tv_sec))(timeout % Async_seconds);
		tp = &ts;
	}

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

AsyncWaitRes Async_wait8(_Atomic u8_lf *addr, u8 val, u64 timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U8);
}

AsyncWaitRes Async_wait16(_Atomic u16_lf *addr, u16 val, u64 timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U16);
}

AsyncWaitRes Async_wait32(_Atomic u32_lf *addr, u32 val, u64 timeout) {
	return Async_ZZwait(addr, val, timeout, FUTEX2_SIZE_U32);
}

AsyncWaitRes Async_wait64(_Atomic u64_lf *addr, u64 val, u64 timeout) {
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

AsyncWaitRes Async_wake8_get(_Atomic u8_lf *addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U8);
}

AsyncWaitRes Async_wake16_get(_Atomic u16_lf *addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U16);
}

AsyncWaitRes Async_wake32_get(_Atomic u32_lf *addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U32);
}

AsyncWaitRes Async_wake64_get(_Atomic u64_lf *addr, int *io_n) {
	return Async_ZZwake(addr, io_n, FUTEX2_SIZE_U64);
}

AsyncWaitRes Async_wake8(_Atomic u8_lf *addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U8);
}

AsyncWaitRes Async_wake16(_Atomic u16_lf *addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U16);
}

AsyncWaitRes Async_wake32(_Atomic u32_lf *addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U32);
}

AsyncWaitRes Async_wake64(_Atomic u64_lf *addr, int n) {
	return Async_ZZwake(addr, &n, FUTEX2_SIZE_U64);
}
