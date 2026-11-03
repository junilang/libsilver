constexpr String String_null = {.data=nullptr,.size=0};

String String_from(Str s) {
	return (String) {.data = (ConstPtr)s, .size = __builtin_strlen(s)};
}

const ubyte *String_data(String this) {
	return this.data;
}

usize String_size(String this) {
	return this.size;
}

bool String_isnull(String this) {
	return this.size == 0
	#if LIBSILVER_SAFE
		|| this.data == nullptr
	#endif
	;
}

OutStreamRes String_print(String this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, this.data, this.size);
}

OutStreamRes StringPtr_print(const String *this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, this->data, this->size);
}

IPrintable_GENERATE_KNOWN(StringPtr, const String*)
