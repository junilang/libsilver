#ifndef Rc_SAFE
	#define Rc_SAFE LIBSILVER_SAFE
#endif

ENUM_GENERATE(RcClass, u8,
	Owner, Weak, Const
)

auto constexpr RcClass_width = ENUM_WIDTH(RcClass);
usize constexpr Rc_classmask = FIELD_MASK_LOWER(usize, RcClass_width);
constexpr usize Rc_ptrmask = ~Rc_classmask;

typedef struct {
	union {
		_Alignas(1ull << RcClass_width) char align__;
		u32 count;
	};
} RcState;

typedef struct {
	union {
		Ptr value;
		usize raw_value;
	};
} Rc;

constexpr Rc ConstRc = { .raw_value = RcClass_Const };

Rc Rc_from(RcState *state, RcClass class) {
	#if Rc_SAFE
		ASSERT(
			!((usize)state & Rc_classmask)
			, "state not aligned"
		);
		ASSERT(class <= ENUM_MAX(RcClass));
	#endif

	return (Rc) {
		.raw_value = ((usize)state & Rc_ptrmask) | (usize)class
	};
}

RcState *Rc_state(Rc this) {
	return (Ptr)(this.raw_value & Rc_ptrmask);
}

RcClass Rc_class(Rc this) {
	auto const class = (RcClass)(this.raw_value & Rc_classmask);
	#if Rc_SAFE
		ASSERT(class <= ENUM_MAX(RcClass));
	#endif

	return class;
}

Rc Rc_init(RcState *state) {
	state->count = 1;
	return Rc_from(state, RcClass_Owner);
}

bool Rc_release(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
		case RcClass_Weak:
			return false;

		case RcClass_Owner:
			auto const state = Rc_state(this);
			return (state->count--) == 1;

		default:
			UNREACHABLE(false);
	}
}

Rc Rc_copy(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
			return this;

		case RcClass_Weak:
		case RcClass_Owner: {
			auto const state = Rc_state(this);
			state->count++;
			return Rc_from(state, RcClass_Owner);
		}

		default:
			UNREACHABLE(false);
	}
}

Rc Rc_weak(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
		case RcClass_Weak:
			return this;

		case RcClass_Owner:
			return Rc_from(Rc_state(this), RcClass_Weak);

		default:
			UNREACHABLE(false);
	}
}

Rc Rc_const(Rc this) {
	switch (Rc_class(this)) {
		case RcClass_Const:
			return this;

		case RcClass_Weak:
		case RcClass_Owner:
			return Rc_from(Rc_state(this), RcClass_Const);

		default:
			UNREACHABLE(false);
	}
}
