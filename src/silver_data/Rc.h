#ifndef Rc_SAFE
	#define Rc_SAFE LIBSILVER_SAFE
#endif

constexpr usize Rc_tagmask = 0b11ull;
constexpr usize Rc_ptrmask = ~Rc_tagmask;

typedef enum : u8 {
	RcClass_Owner,
	RcClass_Weak,
	RcClass_Const
} RcClass;

typedef union {
	_Alignas(Rc_tagmask + 1) char align__;
	u32 count;
} RcState;

typedef union {
	Ptr value;
	usize raw_value;
} Rc;

constexpr Rc ConstRc = { .raw_value = RcClass_Const };

Rc Rc_upcast(RcState *state, RcClass class) {
	return (Rc) {
		.raw_value = ((usize)state & Rc_ptrmask) | (usize)class
	};
}

RcState *Rc_state(Rc this) {
	return (Ptr)(this.raw_value & Rc_ptrmask);
}

RcClass Rc_class(Rc this) {
	return (RcClass)(this.raw_value & Rc_tagmask);
}

bool Rc_valid(Rc this) {
	return Rc_state(this)->count != 0;
}

Rc Rc_init(RcState *state) {
	state->count = 1;
	return Rc_upcast(state, RcClass_Owner);
}

bool Rc_release(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
		case RcClass_Weak:
			return false;

		case RcClass_Owner:
			RcState *state = Rc_state(this);
			return (state->count--) == 1;

		default: UNREACHABLE;
	}
}

Rc Rc_copy(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
			return this;

		#if Rc_SAFE
			case RcClass_Weak: {
				RcState *state = Rc_state(this);
				if (state->count++ == 0)
					PANIC("attempt to copy invalid weak reference");
				return Rc_upcast(state, RcClass_Owner);
			}
		#else
			case RcClass_Weak:
		#endif

		case RcClass_Owner: {
			RcState *state = Rc_state(this);
			state->count++;
			return Rc_upcast(state, RcClass_Owner);
		}

		default: UNREACHABLE;
	}
}

Rc Rc_weak(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
		case RcClass_Weak:
			return this;

		case RcClass_Owner:
			RcState *state = Rc_state(this);
			state->count++;
			return Rc_upcast(state, RcClass_Owner);

		default: UNREACHABLE;
	}
}

Rc Rc_const(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
			return this;
		case RcClass_Weak:
		case RcClass_Owner:
			return Rc_upcast(Rc_state(this), RcClass_Const);

		default: UNREACHABLE;
	}
}
