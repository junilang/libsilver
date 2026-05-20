#if defined(__x86_64__)
	#define Spinlock_PAUSE __builtin_ia32_pause()
	#define Spinlock_TIME __rdtsc()
#else
	#define Spinlock_PAUSE
	#define Spinlock_TIME clock_gettime()
#endif

typedef struct {
	atomic_flag lock;
} Spinlock;

void Spinlock_init(Spinlock *this) {
	atomic_flag_clear(&this->lock);
}

void Spinlock_acquire(Spinlock *this) {
	while (atomic_flag_test_and_set_explicit(&this->lock, memory_order_acquire)) {
		Spinlock_PAUSE;
	}
}

bool Spinlock_timeout(Spinlock *this, u64 timeout) {
	u64 begin = Spinlock_TIME;

	while (atomic_flag_test_and_set_explicit(&this->lock, memory_order_acquire)) {
		Spinlock_PAUSE;
		if (Spinlock_TIME - begin > timeout) return false;
	}

	return true;
}

void Spinlock_sleep(u64 timeout) {
	u64 begin = Spinlock_TIME;

	while (true) {
		Spinlock_PAUSE;
		if (Spinlock_TIME - begin > timeout) return;
	}
}

bool Spinlock_try(Spinlock *this) {
	if (atomic_flag_test_and_set_explicit(&this->lock, memory_order_acquire))
		return false;

	return true;
}

void Spinlock_release(Spinlock *this) {
	atomic_flag_clear_explicit(&this->lock, memory_order_release);
}
