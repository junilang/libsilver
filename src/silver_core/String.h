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

OutStreamRes StringPtr_print(String *this, PrintFmt fmt, OutStream os) {
	return OutStream_write(os, this->data, this->size);
}

IPrintable_GENERATE_KNOWN(StringPtr, String*)
