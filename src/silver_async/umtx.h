#ifdef __linux__
	typedef u32 umtx;

	void umtx_wait(Ptr this, umtx expected_value) {
		syscall(SYS_futex, this, FUTEX_WAIT, expected_value, nullptr);
	}

	#define UMTX_WAKE_ALL INT_MAX

	void umtx_wake(Ptr this, u32 n) {
		syscall(SYS_futex, this, FUTEX_WAKE, n);
	}

#else
	#error "unsupported platform"

#endif
