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

AsyncWaitRes Async_sleep(u64 timeout);

AsyncWaitRes Async_wait8(Ptr addr, u8 expected_value, u64 timeout);
AsyncWaitRes Async_wait16(Ptr addr, u16 expected_value, u64 timeout);
AsyncWaitRes Async_wait32(Ptr addr, u32 expected_value, u64 timeout);
AsyncWaitRes Async_wait64(Ptr addr, u64 expected_value, u64 timeout);

AsyncWaitRes Async_wake8_get(Ptr addr, int *io_n);
AsyncWaitRes Async_wake16_get(Ptr addr, int *io_n);
AsyncWaitRes Async_wake32_get(Ptr addr, int *io_n);
AsyncWaitRes Async_wake64_get(Ptr addr, int *io_n);

AsyncWaitRes Async_wake8(Ptr addr, int n);
AsyncWaitRes Async_wake16(Ptr addr, int n);
AsyncWaitRes Async_wake32(Ptr addr, int n);
AsyncWaitRes Async_wake64(Ptr addr, int n);
