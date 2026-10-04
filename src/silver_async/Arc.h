#ifndef Arc_SAFE
	#define Arc_SAFE BUILD_SAFE
#endif

constexpr usize Arc_tagmask = 0b11ull;
constexpr usize Arc_ptrmask = ~Arc_tagmask;

typedef enum : u8 {
	ArcClass_Owner,
	ArcClass_Weak,
	ArcClass_Const
} ArcClass;

typedef union {
	_Alignas(Arc_tagmask + 1) char align__;
	_Atomic(u32_lf) count;
} ArcState;

typedef union {
	Ptr value;
	usize raw_value;
} Arc;

constexpr Arc ConstArc = { .raw_value = ArcClass_Const };

Arc Arc_upcast(ArcState *state, ArcClass class) {
	return (Arc) {
		.raw_value = ((usize)state & Arc_ptrmask) | (usize)class
	};
}

ArcState *Arc_state(Arc this) {
	return (Ptr)(this.raw_value & Arc_ptrmask);
}

ArcClass Arc_class(Arc this) {
	return (ArcClass)(this.raw_value & Arc_tagmask);
}

bool Arc_valid(Arc this) {
	return atom_get(&Arc_state(this)->count, atom_seq) != 0;
}

Arc Arc_init(ArcState *state) {
	state->count = 1;
	return Arc_upcast(state, ArcClass_Owner);
}

bool Arc_release(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
		case ArcClass_Weak:
			return false;

		case ArcClass_Owner:
			ArcState *state = Arc_state(this);
			return atom_sub(&state->count, 1, atom_seq) == 1;

		default: UNREACHABLE;
	}
}

Arc Arc_copy(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
			return this;

		#if Arc_SAFE
			case ArcClass_Weak: {
				ArcState *state = Arc_state(this);
				if (atom_add(&state->count, 1, atom_seq) == 0)
					PANIC("attempt to copy invalid weak reference");
				return Arc_upcast(state, ArcClass_Owner);
			}
		#else
			case ArcClass_Weak:
		#endif

		case ArcClass_Owner: {
			ArcState *state = Arc_state(this);
			atom_add(&state->count, 1, atom_seq);
			return Arc_upcast(state, ArcClass_Owner);
		}

		default: UNREACHABLE;
	}
}

Arc Arc_weak(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
		case ArcClass_Weak:
			return this;

		case ArcClass_Owner:
			ArcState *state = Arc_state(this);
			atom_add(&state->count, 1, atom_seq);
			return Arc_upcast(state, ArcClass_Owner);

		default: UNREACHABLE;
	}
}

Arc Arc_const(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
			return this;
		case ArcClass_Weak:
		case ArcClass_Owner:
			return Arc_upcast(Arc_state(this), ArcClass_Const);

		default: UNREACHABLE;
	}
}
