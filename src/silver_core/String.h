constexpr String String_null = {.data=nullptr,.size=0};

String String_from(Str s) {
	return (String) {.data = (ConstPtr)s, .size = strlen(s)};
}

uhash String_hash(uhash base, String this) {
	return memhash(base, this.data, this.size);
}

OutStreamRes String_print(String this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, this.data, this.size);
}

typedef struct {
	const ubyte *begin;
	const ubyte *end;
} StringSpan;

#define StringSpan_NULL LITERAL(StringSpan,.begin=nullptr,.end=nullptr)

uhash StringSpan_hash(uhash base, StringSpan this) {
	return memhash(base, this.begin, (usize)(this.end - this.begin));
}

usize StringSpan_size(StringSpan this) {
	return (usize)(this.end - this.begin);
}

OutStreamRes StringSpan_print(StringSpan this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, this.begin, (usize)(this.end - this.begin));
}

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

		return (SmallString){ptrtag((Ptr)data, (utag)size)};
	}

	#define SmallString_NULL ((SmallString){nullptr})
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

	#define SmallString_NULL ((SmallString){.data=nullptr,.size=0})

#endif

uhash SmallString_hash(uhash base, SmallString this) {
	return memhash(base, SmallString_data(this), SmallString_size(this));
}

OutStreamRes SmallString_print(SmallString this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, SmallString_data(this), SmallString_size(this));
}
