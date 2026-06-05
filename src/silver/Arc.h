#define Arc_TAG_BITS 2

typedef union {
	alignas(1ull << Arc_TAG_BITS) char align__;
	struct {
		_Atomic u32 count;
	};
} ArcState;

typedef enum : u8 {
	ArcClass_OWNER,
	ArcClass_WEAK,
	ArcClass_CONST
} ArcClass;

typedef struct {
	Ptr value;
} Arc;

Arc Arc_upcast(ArcState *state, ArcClass class) {
	return (Arc){ .value = lptrtag(state, Arc_TAG_BITS, class) };
}

#define Arc_CONST Arc_upcast(nullptr, ArcClass_CONST);

ArcState *Arc_state(Arc rc) {
	return lptrstrip(rc.value, Arc_TAG_BITS);
}

ArcClass Arc_class(Arc rc) {
	return (ArcClass)lptrread(rc.value, Arc_TAG_BITS);
}

Arc Arc_init(ArcState *state) {
	atomic_init(&state->count, 1);
	state->count = 1;
	return Arc_upcast(state, ArcClass_OWNER);
}

Arc Arc_create(Allocator alc) {
	ArcState *state = Allocator_new(alc, sizeof(ArcState));
	return Arc_init(state);
}

bool Arc_release(Arc rc) {
	switch (Arc_class(rc)) {
		case ArcClass_CONST:
		case ArcClass_WEAK:
			return false;
		case ArcClass_OWNER:
			ArcState *state = Arc_state(rc);
			return atomic_fetch_sub(&state->count, 1) == 1;
	}

	UNREACHABLE;
}

bool Arc_destroy(Arc rc, Allocator alc) {
	switch (Arc_class(rc)) {
		case ArcClass_CONST:
		case ArcClass_WEAK:
			return false;
		case ArcClass_OWNER:
			ArcState *state = Arc_state(rc);
			if (atomic_fetch_sub(&state->count, 1) != 1)
				return false;

			Allocator_delete(alc, state);
			return true;
	}

	UNREACHABLE;
}

Arc Arc_copy(Arc rc) {
	switch (Arc_class(rc)) {
		case ArcClass_CONST:
			return rc;
		case ArcClass_WEAK:
		case ArcClass_OWNER:
			ArcState *state = Arc_state(rc);
			atomic_fetch_add(&state->count, 1);
			return Arc_upcast(state, ArcClass_OWNER);
	}

	UNREACHABLE;
}

Arc Arc_weak(Arc rc) {
	switch (Arc_class(rc)) {
		case ArcClass_CONST:
		case ArcClass_WEAK:
			return rc;
		case ArcClass_OWNER:
			return Arc_upcast(Arc_state(rc), ArcClass_WEAK);
	}

	UNREACHABLE;
}

Arc Arc_const(Arc rc) {
	switch (Arc_class(rc)) {
		case ArcClass_CONST:
		case ArcClass_WEAK:
			return rc;
		case ArcClass_OWNER:
			return Arc_upcast(Arc_state(rc), ArcClass_WEAK);
	}

	UNREACHABLE;
}

bool Arc_isconst(Arc rc) { return Arc_class(rc) == ArcClass_CONST; }
bool Arc_isweak(Arc rc) { return Arc_class(rc) == ArcClass_WEAK; }
bool Arc_isowner(Arc rc) { return Arc_class(rc) == ArcClass_OWNER; }
