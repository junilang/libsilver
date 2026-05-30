typedef struct {
	u64 capacity;
	u64 size;

	alignas(RioTask_ALIGNMENT) ubyte data[];
} RioQueue;

usize ZZRioQueue_allocsize(usize size) {
	return usize_pflx()
}

usize ZZRioQueue_nextcapacity(usize capacity, usize target) {

}

#define RioQinfo_BITS 64

#define RioQinfo_LOCK_BITS 1
#define RioQinfo_LOCK (1ull)

#define RioQinfo_RC_BITS 12
#define RioQinfo_RC_SHIFT RioQinfo_LOCK_BITS
#define RioQinfo_RC_MAX ((1ull << RioQinfo_RC_BITS) - 1)
#define RioQinfo_RC_MASK (RioQinfo_RC_MAX << RioQinfo_RC_SHIFT)
#define RioQinfo_RC_ONE (1ull << RioQinfo_RC_SHIFT)

#define RioQinfo_POS_SHIFT (RioQinfo_RC_SHIFT + RioQinfo_RC_BITS)
#define RioQinfo_POS_BITS (RioQinfo_BITS - RioQinfo_POS_SHIFT)
#define RioQinfo_POS_MASK (((1ull << RioQinfo_POS_BITS) - 1) << RioQinfo_POS_SHIFT)
#define RioQinfo_POS_ONE (1ull << RioQinfo_POS_SHIFT)

typedef u64 RioQinfo;

usize RioQinfo_pos(RioQinfo this) {
	return (this & RioQinfo_POS_MASK) >> RioQinfo_POS_SHIFT;
}

usize ZZRio_nextcapacity(usize capacity, usize target_capacity) {
	while (capacity < target_capacity) {
		capacity += capacity / 2;
	}
	return capacity;
}
