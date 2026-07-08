#if __clang__
	#define atom_acq __ATOMIC_ACQUIRE
	#define atom_rel __ATOMIC_RELEASE

	#define atom_sync __ATOMIC_ACQ_REL
	#define atom_acqrel __ATOMIC_ACQ_REL

	#define atom_lazy __ATOMIC_RELAXED
	#define atom_relaxed __ATOMIC_RELAXED

	#define atom_seq __ATOMIC_SEQ_CST
	#define atom_seqcst __ATOMIC_SEQ_CST

	#define atom_consume __ATOMIC_CONSUME

#else
	#define atom_acq __ATOMIC_ACQUIRE__
	#define atom_rel __ATOMIC_RELEASE__

	#define atom_sync __ATOMIC_ACQUIRE_RELEASE__
	#define atom_acqrel __ATOMIC_ACQUIRE_RELEASE__

	#define atom_lazy __ATOMIC_RELAXED__
	#define atom_relaxed __ATOMIC_RELAXED__

	#define atom_seq __ATOMIC_SEQ_CST__
	#define atom_seqcst __ATOMIC_SEQ_CST__

	#define atom_consume __ATOMIC_CONSUME__

#endif

#define atom_get(object, memory_order) \
	__c11_atomic_load(object, memory_order)
#define atom_set(object, value, memory_order) \
	__c11_atomic_store(object, value, memory_order)

#define atom_cmpx \
	__c11_atomic_compare_exchange_strong
#define atom_cmpx_weak \
	__c11_atomic_compare_exchange_weak

#define atom_add __c11_atomic_fetch_add
#define atom_sub __c11_atomic_fetch_sub
#define atom_or __c11_atomic_fetch_or
#define atom_xor __c11_atomic_fetch_xor
#define atom_and __c11_atomic_fetch_and

#define atom_fence __c11_atomic_thread_fence
#define atom_sigfence __c11_atomic_signal_fence
#define atom_killdep __c11_atomic_kill_dependency
