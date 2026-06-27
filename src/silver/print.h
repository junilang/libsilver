#define PRINT(stream, ...) { __VA_OPT__(\
	const OutStream PRINT_stream__ = (stream); \
	PrintFmt PRINT_fmt__ = PrintFmt_Null; \
	PRINT_0(PRINT_stream__, (&PRINT_fmt__), __VA_ARGS__) \
) }

#define PRINT_BUFFERED(buffer_size, stream, ...) { __VA_OPT__( \
	constexpr usize PRINT_buffer_size__ = (buffer_size); \
	const OutStream PRINT_stream__ = (stream); \
	ubyte PRINT_buffer_data__[PRINT_buffer_size__]; \
	BufferOutStream PRINT_buffer__ = {.buffer=PRINT_buffer_data__,.capacity=PRINT_buffer_size__}; \
	const OutStream PRINT_buffer_os__ = BufferOutStream_upcast(&PRINT_buffer__); \
	PrintFmt PRINT_fmt__ = PrintFmt_Null; \
	PRINT_0(PRINT_buffer_os__, (&PRINT_fmt__), __VA_ARGS__) \
	if (OutStream_write(PRINT_stream__, PRINT_buffer_data__, PRINT_buffer__.size)) \
		PANIC("PRINT_BUFFERED write failed"); \
) }

#define PRINTB PRINT_BUFFERED

#define PRINT_ITEM(S, F, V) _Generic((V), \
	PrintFmt : PRINT_setfmt, \
	char * : PRINT_cstring, \
	const char * : PRINT_cstring, \
	bool : PRINT_bool, \
	Printable : PRINT_Printable, \
	StaticPrintable : PRINT_StaticPrintable, \
	FmtPrintable : PRINT_FmtPrintable, \
	StaticFmtPrintable : PRINT_StaticFmtPrintable, \
	String : PRINT_String, \
	SmallString : PRINT_SmallString, \
	StringSpan : PRINT_StringSpan, \
	signed char : PRINT_IntFmt_Signed, \
	signed short : PRINT_IntFmt_Signed, \
	signed int : PRINT_IntFmt_Signed, \
	signed long : PRINT_IntFmt_Signed, \
	signed long long : PRINT_IntFmt_Signed, \
	unsigned char : PRINT_IntFmt_Unsigned,  \
	unsigned short : PRINT_IntFmt_Unsigned, \
	unsigned int : PRINT_IntFmt_Unsigned, \
	unsigned long : PRINT_IntFmt_Unsigned, \
	unsigned long long : PRINT_IntFmt_Unsigned, \
	default : PRINT_pointer \
)(S, F, (V));

void PRINT_setfmt(OutStream os, PrintFmt *fmt, PrintFmt fmt_set) {
	*fmt = fmt_set;
}

void PRINT_cstring(OutStream os, PrintFmt *fmt, const char *v) {
	if (String_print(STRING(v), *fmt, os)) PANIC("String_print failed");
	*fmt = PrintFmt_Null;
}

void PRINT_bool(OutStream os, PrintFmt *fmt, bool v) {
	const char *repr;
	usize size;

	if (v) {
		repr = "true";
		size = 4;
	} else {
		repr = "false";
		size = 5;
	}

	if (OutStream_write(os, repr, size)) {
		PANIC("failed");
	}

	*fmt = PrintFmt_Null;
}

void PRINT_pointer(OutStream os, PrintFmt *fmtp, ConstPtr v) {
	PrintFmt fmt = *fmtp;
	if (!fmt.value)
		fmt.value = FIELD_SET(IntFmt_Base, Hex) | FLAG(IntFmt, Header);
	IntFmt_Unsigned_print((usize)v, fmt, os);
	*fmtp = PrintFmt_Null;
}

#define PRINT_GENERATE(T) \
	void PRINT_##T(OutStream os, PrintFmt *fmt, T v) { \
		if (T##_print(v, *fmt, os)) PANIC(#T"_print failed"); \
		*fmt = PrintFmt_Null; \
	}

PRINT_GENERATE(IntFmt_Unsigned)
PRINT_GENERATE(IntFmt_Signed)
PRINT_GENERATE(String)
PRINT_GENERATE(StringSpan)
PRINT_GENERATE(SmallString)
PRINT_GENERATE(Printable)
PRINT_GENERATE(StaticPrintable)
PRINT_GENERATE(FmtPrintable)
PRINT_GENERATE(StaticFmtPrintable)

#define PRINT_0(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_1(S, F, __VA_ARGS__))
#define PRINT_1(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_2(S, F, __VA_ARGS__))
#define PRINT_2(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_3(S, F, __VA_ARGS__))
#define PRINT_3(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_4(S, F, __VA_ARGS__))
#define PRINT_4(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_5(S, F, __VA_ARGS__))
#define PRINT_5(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_6(S, F, __VA_ARGS__))
#define PRINT_6(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_7(S, F, __VA_ARGS__))
#define PRINT_7(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_8(S, F, __VA_ARGS__))
#define PRINT_8(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_9(S, F, __VA_ARGS__))
#define PRINT_9(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_10(S, F, __VA_ARGS__))
#define PRINT_10(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_11(S, F, __VA_ARGS__))
#define PRINT_11(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_12(S, F, __VA_ARGS__))
#define PRINT_12(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_13(S, F, __VA_ARGS__))
#define PRINT_13(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_14(S, F, __VA_ARGS__))
#define PRINT_14(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_15(S, F, __VA_ARGS__))
#define PRINT_15(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_16(S, F, __VA_ARGS__))
#define PRINT_16(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_17(S, F, __VA_ARGS__))
#define PRINT_17(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_18(S, F, __VA_ARGS__))
#define PRINT_18(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_19(S, F, __VA_ARGS__))
#define PRINT_19(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_20(S, F, __VA_ARGS__))
#define PRINT_20(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_21(S, F, __VA_ARGS__))
#define PRINT_21(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_22(S, F, __VA_ARGS__))
#define PRINT_22(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_23(S, F, __VA_ARGS__))
#define PRINT_23(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_24(S, F, __VA_ARGS__))
#define PRINT_24(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_25(S, F, __VA_ARGS__))
#define PRINT_25(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_26(S, F, __VA_ARGS__))
#define PRINT_26(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_27(S, F, __VA_ARGS__))
#define PRINT_27(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_28(S, F, __VA_ARGS__))
#define PRINT_28(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_29(S, F, __VA_ARGS__))
#define PRINT_29(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_30(S, F, __VA_ARGS__))
#define PRINT_30(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_31(S, F, __VA_ARGS__))
#define PRINT_31(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_32(S, F, __VA_ARGS__))
#define PRINT_32(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_33(S, F, __VA_ARGS__))
#define PRINT_33(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_34(S, F, __VA_ARGS__))
#define PRINT_34(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_35(S, F, __VA_ARGS__))
#define PRINT_35(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_36(S, F, __VA_ARGS__))
#define PRINT_36(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_37(S, F, __VA_ARGS__))
#define PRINT_37(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_38(S, F, __VA_ARGS__))
#define PRINT_38(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_39(S, F, __VA_ARGS__))
#define PRINT_39(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_40(S, F, __VA_ARGS__))
#define PRINT_40(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_41(S, F, __VA_ARGS__))
#define PRINT_41(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_42(S, F, __VA_ARGS__))
#define PRINT_42(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_43(S, F, __VA_ARGS__))
#define PRINT_43(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_44(S, F, __VA_ARGS__))
#define PRINT_44(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_45(S, F, __VA_ARGS__))
#define PRINT_45(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_46(S, F, __VA_ARGS__))
#define PRINT_46(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_47(S, F, __VA_ARGS__))
#define PRINT_47(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_48(S, F, __VA_ARGS__))
#define PRINT_48(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_49(S, F, __VA_ARGS__))
#define PRINT_49(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_50(S, F, __VA_ARGS__))
#define PRINT_50(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_51(S, F, __VA_ARGS__))
#define PRINT_51(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_52(S, F, __VA_ARGS__))
#define PRINT_52(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_53(S, F, __VA_ARGS__))
#define PRINT_53(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_54(S, F, __VA_ARGS__))
#define PRINT_54(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_55(S, F, __VA_ARGS__))
#define PRINT_55(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_56(S, F, __VA_ARGS__))
#define PRINT_56(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_57(S, F, __VA_ARGS__))
#define PRINT_57(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_58(S, F, __VA_ARGS__))
#define PRINT_58(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_59(S, F, __VA_ARGS__))
#define PRINT_59(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_60(S, F, __VA_ARGS__))
#define PRINT_60(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_61(S, F, __VA_ARGS__))
#define PRINT_61(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_62(S, F, __VA_ARGS__))
#define PRINT_62(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_63(S, F, __VA_ARGS__))
#define PRINT_63(S, F, V, ...) PRINT_ITEM(S, F, (V)); __VA_OPT__(PRINT_64(S, F, __VA_ARGS__))
#define PRINT_64(S, F, V, ...) PRINT_TOO_MANY_ITEMS
