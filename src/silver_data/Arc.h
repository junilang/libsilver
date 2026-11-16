#ifndef Arc_SAFE
	#define Arc_SAFE LIBSILVER_SAFE
#endif

ENUM_GENERATE(ArcClass, u8,
	Owner, Weak, Const
)

auto constexpr ArcClass_width = ENUM_WIDTH(ArcClass);
usize constexpr Arc_classmask = FIELD_MASK_LOWER(usize, ArcClass_width);
constexpr usize Arc_ptrmask = ~Arc_classmask;

typedef struct {
	union {
		_Alignas(1ull << ArcClass_width) char align__;
		_Atomic u32_lf count;
	};
} ArcState;

typedef struct {
	union {
		Ptr value;
		usize raw_value;
	};
} Arc;

constexpr Arc ConstArc = { .raw_value = ArcClass_Const };

Arc Arc_from(ArcState *state, ArcClass class) {
	#if Arc_SAFE
		ASSERT(
			!((usize)state & Arc_classmask)
			, "state not aligned"
		);
		ASSERT(class <= ENUM_MAX(ArcClass));
	#endif

	return (Arc) {
		.raw_value = ((usize)state & Arc_ptrmask) | (usize)class
	};
}

ArcState *Arc_state(Arc this) {
	return (Ptr)(this.raw_value & Arc_ptrmask);
}

ArcClass Arc_class(Arc this) {
	auto const class = (ArcClass)(this.raw_value & Arc_classmask);
	#if Arc_SAFE
		ASSERT(class <= ENUM_MAX(ArcClass));
	#endif

	return class;
}

Arc Arc_init(ArcState *state) {
	state->count = 1;
	return Arc_from(state, ArcClass_Owner);
}

bool Arc_release(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
		case ArcClass_Weak:
			return false;

		case ArcClass_Owner:
			auto const state = Arc_state(this);
			return atom_sub(&state->count, 1, atom_sync) == 1;

		default:
			UNREACHABLE(false);
	}
}

Arc Arc_copy(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
			return this;

		case ArcClass_Weak:
		case ArcClass_Owner: {
			auto const state = Arc_state(this);
			atom_add(&state->count, 1, atom_sync);
			return Arc_from(state, ArcClass_Owner);
		}

		default:
			UNREACHABLE(false);
	}
}

Arc Arc_weak(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
		case ArcClass_Weak:
			return this;

		case ArcClass_Owner:
			return Arc_from(Arc_state(this), ArcClass_Weak);

		default:
			UNREACHABLE(false);
	}
}

Arc Arc_const(Arc this) {
	switch (Arc_class(this)) {
		case ArcClass_Const:
			return this;

		case ArcClass_Weak:
		case ArcClass_Owner:
			return Arc_from(Arc_state(this), ArcClass_Const);

		default:
			UNREACHABLE(false);
	}
}
