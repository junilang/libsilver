#define XS \
	X(Ok) \
	X(Timeout) \
	X(Mismatch) \
	X(Interrupt) \
	X(ErrInvalid) \
	X(ErrClock) \
	X(ErrUnknown)


typedef enum : u8 {
	#define X(N) AsyncWaitRes_##N,
		XS
	#undef X
} AsyncWaitRes;

const String AsyncWaitRes_repr[] = {
	#define X(N) [AsyncWaitRes_##N] = String_INIT("AsyncWaitRes_"#N),
		XS
	#undef X
};

#undef XS

constexpr u64 Async_seconds = 1'000'0000'000ull;
constexpr u64 Async_millis = 1'000'000ull;
constexpr u64 Async_micros = 1'000ull;
constexpr u64 Async_nanos = 1ull;

AsyncWaitRes Async_wait8(_Atomic u8_lf *addr, u8 expected_value, u64 timeout);
AsyncWaitRes Async_wait16(_Atomic u16_lf *addr, u16 expected_value, u64 timeout);
AsyncWaitRes Async_wait32(_Atomic u32_lf *addr, u32 expected_value, u64 timeout);
AsyncWaitRes Async_wait64(_Atomic u64_lf *addr, u64 expected_value, u64 timeout);

AsyncWaitRes Async_wake8_get(_Atomic u8_lf *addr, int *io_n);
AsyncWaitRes Async_wake16_get(_Atomic u16_lf *addr, int *io_n);
AsyncWaitRes Async_wake32_get(_Atomic u32_lf *addr, int *io_n);
AsyncWaitRes Async_wake64_get(_Atomic u64_lf *addr, int *io_n);

AsyncWaitRes Async_wake8(_Atomic u8_lf *addr, int n);
AsyncWaitRes Async_wake16(_Atomic u16_lf *addr, int n);
AsyncWaitRes Async_wake32(_Atomic u32_lf *addr, int n);
AsyncWaitRes Async_wake64(_Atomic u64_lf *addr, int n);
