#define atom_acq __ATOMIC_ACQUIRE
#define atom_rel __ATOMIC_RELEASE

#define atom_sync __ATOMIC_ACQ_REL
#define atom_acqrel __ATOMIC_ACQ_REL

#define atom_lazy __ATOMIC_RELAXED
#define atom_relaxed __ATOMIC_RELAXED

#define atom_seq __ATOMIC_SEQ_CST
#define atom_seqcst __ATOMIC_SEQ_CST

#define atom_consume __ATOMIC_CONSUME

#if __clang__
	#define atom_init __c11_atomic_init

	#define atom_get __c11_atomic_load
	#define atom_set __c11_atomic_store

	#define atom_exchange __c11_atomic_exchange
	#define atom_exchg __c11_atomic_exchange

	#define atom_cmpx __c11_atomic_compare_exchange_strong
	#define atom_cmpx_weak __c11_atomic_compare_exchange_weak

	#define atom_add __c11_atomic_fetch_add
	#define atom_sub __c11_atomic_fetch_sub
	#define atom_or __c11_atomic_fetch_or
	#define atom_xor __c11_atomic_fetch_xor
	#define atom_and __c11_atomic_fetch_and

	#define atom_fence __c11_atomic_thread_fence
	#define atom_sigfence __c11_atomic_signal_fence
	#define atom_killdep __c11_atomic_kill_dependency

#else
	#define atom_init __atomic_init

	#define atom_get __atomic_load_n
	#define atom_set __atomic_store_n

	#define atom_exchange __atomic_exchange_n
	#define atom_exchg __atomic_exchange_n

	#define atom_cmpx(ptr, expected, desired, mo_success, mo_fail) \
		__atomic_compare_exchange_n((ptr), (expected), (desired), false, (mo_success), (mo_fail))

	#define atom_cmpx_weak \
		__atomic_compare_exchange_n((ptr), (expected), (desired), true, (mo_success), (mo_fail))

	#define atom_add __atomic_fetch_add
	#define atom_sub __atomic_fetch_sub
	#define atom_or __atomic_fetch_or
	#define atom_xor __atomic_fetch_xor
	#define atom_and __atomic_fetch_and

	#define atom_fence __atomic_thread_fence
	#define atom_sigfence __atomic_signal_fence
	#define atom_killdep __atomic_kill_dependency

#endif
