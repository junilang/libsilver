#ifndef SmallString_PTRTAG
	#define SmallString_PTRTAG PTRTAG
#endif

#ifndef SmallString_SAFE
	#define SmallString_SAFE BUILD_SAFE
#endif

#if SmallString_PTRTAG

	#define SmallString_MAX PTRTAG_MAX

	typedef struct {
		Ptr value;
	} SmallString;

	usize SmallString_size(SmallString this) {
		return ptrread(this.value);
	}

	const ubyte *SmallString_data(SmallString this) {
		return ptrstrip(this.value);
	}

	SmallString SmallString_upcast(const ubyte *data, usize size) {
		#if SmallString_SAFE
			if (size > SmallString_MAX) PANIC("size overflow");
		#endif

		return (SmallString){.value=ptrtag((Ptr)data, (utag)size)};
	}

	constexpr SmallString SmallString_null = {.value=nullptr};

	#define SmallString_INIT(str) {.value=CONSTEXPR_ptrtag((ConstPtr)(str), (utag)(sizeof(str) - 1))}

#else

	#define SmallString_MAX SIZE_MAX

	[[deprecated("pointer tagging disabled - SmallString = String")]]
	typedef struct {
		const ubyte *data;
		usize size;
	} SmallString;

	usize SmallString_size(SmallString this) {
		return this.size;
	}

	const ubyte *SmallString_data(SmallString this) {
		return this.data;
	}

	[[deprecated("pointer tagging disabled - SmallString = String")]]
	SmallString SmallString_upcast(const ubyte *data, usize size) {
		return (SmallString){.data=data,.size=size};
	}

	constexpr SmallString SmallString_null = {.data=nullptr,.size=0};

	#define SmallString_INIT(str) {.data=(ConstPtr)(str), .size=(sizeof(str) - 1)}

#endif

uhash SmallString_hash(uhash base, SmallString this) {
	return memhash(base, SmallString_data(this), SmallString_size(this));
}

OutStreamRes SmallString_print(SmallString this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, SmallString_data(this), SmallString_size(this));
}

String SmallString_tostr(SmallString this) {
	return (String) {
		.data = SmallString_data(this),
		.size = SmallString_size(this)
	};
}
