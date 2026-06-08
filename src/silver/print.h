/*
void PRINT_bool(bool value, OutStream os) {
	if (value)
		OutStream_write(os, USTR("true"));
	else
		OutStream_write(os, USTR("false"));
}

void PRINT_char(char value, OutStream os) {
	OutStream_write(os, (ubyte*)&value, 1);
}

void PRINT_cstring(Str cstr, OutStream os) {
	if (!cstr) {
		OutStream_write(os, USTR("(nullstr)"));
	}

	OutStream_write(os, (const ubyte*)cstr, __builtin_strlen(cstr));
}

#define PRINT_ITEM(S, A) _Generic((A), \
	char : PRINT_char, \
	unsigned char : PRINT_uchar, \
	short : PRINT_short, \
	unsigned short : PRINT_ushort, \
	int : PRINT_int, \
	unsigned int : PRINT_uint, \
	long : PRINT_long, \
	unsigned long : PRINT_ulong, \
	long long : PRINT_llong, \
	unsigned long long : PRINT_ullong, \
	float : PRINT_float, \
	double : PRINT_double, \
	long double : PRINT_ldouble, \
	bool : PRINT_bool, \
	char* : PRINT_cstring, \
	Str : PRINT_cstring, \
	Ptr : PRINT_ptr, \
	String : String_print, \
	StringSpan : StringSpan_print, \
	SmallString : SmallString_print, \
	VString : VString_print, \
	Printable : Printable_print, \
	default : PRINT_ptr \
)((A), (S))

*/

#define PRINT_X(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(GCC_ERROR_MAX_DEPTH_REACHED)

#define PRINT_1(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_2(S, __VA_ARGS__))
#define PRINT_2(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_3(S, __VA_ARGS__))
#define PRINT_3(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_4(S, __VA_ARGS__))
#define PRINT_4(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_5(S, __VA_ARGS__))
#define PRINT_5(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_6(S, __VA_ARGS__))
#define PRINT_6(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_7(S, __VA_ARGS__))
#define PRINT_7(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_8(S, __VA_ARGS__))
#define PRINT_8(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_9(S, __VA_ARGS__))
#define PRINT_9(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_10(S, __VA_ARGS__))
#define PRINT_10(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_11(S, __VA_ARGS__))
#define PRINT_11(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_12(S, __VA_ARGS__))
#define PRINT_12(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_13(S, __VA_ARGS__))
#define PRINT_13(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_14(S, __VA_ARGS__))
#define PRINT_14(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_15(S, __VA_ARGS__))
#define PRINT_15(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_16(S, __VA_ARGS__))
#define PRINT_16(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_17(S, __VA_ARGS__))
#define PRINT_17(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_18(S, __VA_ARGS__))
#define PRINT_18(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_19(S, __VA_ARGS__))
#define PRINT_19(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_X(S, __VA_ARGS__))

#define PRINT(stream, ...) { \
	const OutStream PRINT__stream = (stream); \
	PRINT_1(PRINT__stream, __VA_ARGS__) \
}

#ifndef PRINT_ATOMIC_BUFFER_SIZE
	#define PRINT_ATOMIC_BUFFER_SIZE 256
#endif

//#define FPRINT(file, ...) PRINT(FileOutStream_upcast(file), __VA_ARGS__)

#define PRINT_ATOMIC_(bs, os, ...) { \
	ubyte PRINT__buffer[bs]; \
	BufferOutStream PRINT__os = {.buffer=PRINT__buffer,.capacity=bs}; \
	PRINT(BufferOutStream_upcast(&PRINT__os), __VA_ARGS__); \
	OutStream_write(os, PRINT__buffer, PRINT__os.size); \
}

//#define FPRINT_ATOMIC_(bs, file, ...) PRINT_ATOMIC_(bs, FileOutStream_upcast(file), __VA_ARGS__)

#define PRINT_ATOMIC(os, ...) PRINT_ATOMIC_(PRINT_ATOMIC_BUFFER_SIZE, os, __VA_ARGS__)
//#define FPRINT_ATOMIC(file, ...) FPRINT_ATOMIC_(PRINT_ATOMIC_BUFFER_SIZE, file, __VA_ARGS__)
