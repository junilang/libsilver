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

constexpr StringSpan StringSpan_null = {.begin=nullptr,.end=nullptr};

uhash StringSpan_hash(uhash base, StringSpan this) {
	return memhash(base, this.begin, (usize)(this.end - this.begin));
}

usize StringSpan_size(StringSpan this) {
	return (usize)(this.end - this.begin);
}

OutStreamRes StringSpan_print(StringSpan this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, this.begin, (usize)(this.end - this.begin));
}
