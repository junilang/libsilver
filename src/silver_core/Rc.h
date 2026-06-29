constexpr u8 Rc_Tagbits = 2;

typedef union {
	alignas(1ull << Rc_Tagbits) char align__;
	struct {
		u32 count;
	};
} RcState;

typedef enum : u8 {
	RcClass_Owner,
	RcClass_Weak,
	RcClass_Const
} RcClass;

typedef union {
	Ptr value;
	usize raw_value;
} Rc;

Rc Rc_setclass(RcState *state, RcClass class) {
	return (Rc){.value=lptrtag(state, Rc_Tagbits, class)};
}

constexpr Rc Rc_Const = {.raw_value=(RcClass_Const)};

RcState *Rc_state(Rc this) {
	return lptrstrip(this.value, Rc_Tagbits);
}

RcClass Rc_class(Rc rc) {
	return (RcClass)lptrread(rc, Rc_BITS);
}

Rc Rc_init(RcState *state) {
	state->count = 1;
	return ZZRc_setclass(state, RcClass_OWNER);
}

Rc Rc_create(Allocator alc) {
	RcState *state = Allocator_new(alc, sizeof(RcState));
	return Rc_init(state);
}

bool Rc_release(Rc rc) {
	switch (Rc_class(rc)) {
		case RcClass_CONST:
		case RcClass_WEAK:
			return false;
		case RcClass_OWNER:
			RcState *state = Rc_state(rc);
			return state->count-- == 1;
	} UNREACHABLE;
}

bool Rc_destroy(Rc rc, Allocator alc) {
	switch (Rc_class(rc)) {
		case RcClass_CONST:
		case RcClass_WEAK:
			return false;
		case RcClass_OWNER:
			RcState *state = Rc_state(rc);
			if (state->count-- != 1)
				return false;

			Allocator_delete(alc, state);
			return true;
	} UNREACHABLE;
}

Rc Rc_copy(Rc rc) {
	switch (Rc_class(rc)) {
		case RcClass_CONST:
			return rc;
		case RcClass_WEAK:
		case RcClass_OWNER:
			RcState *state = Rc_state(rc);
			state->count++;
			return ZZRc_setclass(state, RcClass_OWNER);
	} UNREACHABLE;
}

Rc Rc_weak(Rc rc) {
	switch (Rc_class(rc)) {
		case RcClass_CONST:
		case RcClass_WEAK:
			return rc;
		case RcClass_OWNER:
			return ZZRc_setclass(Rc_state(rc), RcClass_WEAK);
	} UNREACHABLE;
}

Rc Rc_const(Rc rc) {
	switch (Rc_class(rc)) {
		case RcClass_CONST:
		case RcClass_WEAK:
			return rc;
		case RcClass_OWNER:
			return ZZRc_setclass(Rc_state(rc), RcClass_WEAK);
	} UNREACHABLE;
}

bool Rc_isconst(Rc rc) { return Rc_class(rc) == RcClass_CONST; }
bool Rc_isweak(Rc rc) { return Rc_class(rc) == RcClass_WEAK; }
bool Rc_isowner(Rc rc) { return Rc_class(rc) == RcClass_OWNER; }
