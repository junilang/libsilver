typedef struct {
	const ubyte *data;
	usize size;
} String;

#define String_INIT(str) {.data=(ConstPtr)(str), .size=(__builtin_strlen(str))}
#define String_FROM(str) ((String)String_INIT(str))

constexpr String String_null = {.data=nullptr,.size=0};

String String_from(Str s) {
	auto const size = __builtin_strlen(s);
	return (String) {.data = (ConstPtr)s, .size = size};
}

String String_create(ConstPtr data, usize size) {
	return (String) { .data = data, .size = size };
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
