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

String StringSpan_tostr(StringSpan this) {
	return (String) {
		.data = this.begin,
		.size = StringSpan_size(this)
	};
}
