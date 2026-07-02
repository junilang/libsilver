#define PRINT(resp, stream, ...) { \
	const OutStream PRINT_stream__ = (stream); \
	OutStreamRes PRINT_res__ = OutStreamRes_Ok; \
	PrintFmt PRINT_fmt__ = PrintFmt_Null; \
	do { \
		__VA_OPT__(PRINT_0(__VA_ARGS__)) \
	} while (0); \
	*(resp) = PRINT_res__; \
}

#define PRINT_PANIC(stream, ...) { \
	OutStreamRes PRINT_outer_res__; \
	PRINT(&PRINT_outer_res__, stream, __VA_ARGS__) \
	if (PRINT_outer_res__) PANIC("PRINT failed"); \
}

#define PRINTP PRINT_PANIC

#define PRINT_BUFFERED(resp, buffer_size, stream, ...) { \
	constexpr usize PRINT_buffer_size__ = (buffer_size); \
	const OutStream PRINT_parent_stream__ = (stream); \
	ubyte PRINT_buffer__[PRINT_buffer_size__]; \
	BufferOutStream PRINT_buffer_os__ = {.buffer=PRINT_buffer__,.capacity=PRINT_buffer_size__}; \
	const OutStream PRINT_stream__ = BufferOutStream_upcast(&PRINT_buffer_os__); \
	OutStreamRes PRINT_res__ = OutStreamRes_Ok; \
	PrintFmt PRINT_fmt__ = PrintFmt_Null; \
	do { \
		__VA_OPT__(PRINT_0(__VA_ARGS__)) \
		PRINT_res__ = OutStream_write( \
			PRINT_parent_stream__, PRINT_buffer__, PRINT_buffer_os__.size \
		); \
	} while (0); \
	*(resp) = PRINT_res__; \
}

#define PRINTB PRINT_BUFFERED

#define PRINT_BUFFERED_PANIC(buffer_size, stream, ...) { \
	OutStreamRes PRINT_outer_res__; \
	PRINT_BUFFERED(&PRINT_outer_res__, buffer_size, stream, __VA_ARGS__); \
	if (PRINT_outer_res__) PANIC("PRINT failed"); \
}

#define PRINTBP PRINT_BUFFERED_PANIC

#define PANIC_PRINT(...) { \
	PANIC_HEADER \
	OutStreamRes PRINT_outer_res__; \
	PRINT(&PRINT_outer_res__, os_panic_stream(), __VA_ARGS__, "\n"); \
	if (PRINT_outer_res__) os_panic_write(STR("\nPANIC_PRINT failed\n")); \
	os_panic(); \
}

#define PRINT_ITEM(V) { \
	PRINT_res__ = _Generic((V), \
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
	)(PRINT_stream__, &PRINT_fmt__, (V)); \
	if (PRINT_res__) break; \
}

OutStreamRes PRINT_setfmt(OutStream os, PrintFmt *fmtp, PrintFmt fmt) {
	*fmtp = fmt;
	return OutStreamRes_Ok;
}

OutStreamRes PRINT_cstring(OutStream os, PrintFmt *fmtp, Str v) {
	PrintFmt fmt = *fmtp;
	*fmtp = PrintFmt_Null;
	return String_print(String_from(v), fmt, os);
}

OutStreamRes PRINT_bool(OutStream os, PrintFmt *fmt, bool v) {
	*fmt = PrintFmt_Null;

	Str repr;
	usize size;

	if (v) {
		repr = "true";
		size = 4;
	} else {
		repr = "false";
		size = 5;
	}

	return OutStream_write(os, repr, size);
}

OutStreamRes PRINT_pointer(OutStream os, PrintFmt *fmtp, ConstPtr v) {
	PrintFmt fmt = *fmtp;
	*fmtp = PrintFmt_Null;
	if (!fmt.value)
		fmt.value = FIELD(IntFmt_Base, Hex) | FLAG(IntFmt_Header);
	return IntFmt_Unsigned_print((usize)v, fmt, os);
}

#define PRINT_GENERATE(T) \
	OutStreamRes PRINT_##T(OutStream os, PrintFmt *fmtp, T v) { \
		PrintFmt fmt = *fmtp; \
		*fmtp = PrintFmt_Null; \
		return T##_print(v, fmt, os); \
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

#define PRINT_0(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_1(__VA_ARGS__))
#define PRINT_1(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_2(__VA_ARGS__))
#define PRINT_2(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_3(__VA_ARGS__))
#define PRINT_3(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_4(__VA_ARGS__))
#define PRINT_4(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_5(__VA_ARGS__))
#define PRINT_5(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_6(__VA_ARGS__))
#define PRINT_6(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_7(__VA_ARGS__))
#define PRINT_7(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_8(__VA_ARGS__))
#define PRINT_8(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_9(__VA_ARGS__))
#define PRINT_9(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_10(__VA_ARGS__))
#define PRINT_10(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_11(__VA_ARGS__))
#define PRINT_11(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_12(__VA_ARGS__))
#define PRINT_12(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_13(__VA_ARGS__))
#define PRINT_13(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_14(__VA_ARGS__))
#define PRINT_14(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_15(__VA_ARGS__))
#define PRINT_15(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_16(__VA_ARGS__))
#define PRINT_16(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_17(__VA_ARGS__))
#define PRINT_17(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_18(__VA_ARGS__))
#define PRINT_18(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_19(__VA_ARGS__))
#define PRINT_19(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_20(__VA_ARGS__))
#define PRINT_20(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_21(__VA_ARGS__))
#define PRINT_21(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_22(__VA_ARGS__))
#define PRINT_22(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_23(__VA_ARGS__))
#define PRINT_23(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_24(__VA_ARGS__))
#define PRINT_24(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_25(__VA_ARGS__))
#define PRINT_25(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_26(__VA_ARGS__))
#define PRINT_26(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_27(__VA_ARGS__))
#define PRINT_27(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_28(__VA_ARGS__))
#define PRINT_28(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_29(__VA_ARGS__))
#define PRINT_29(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_30(__VA_ARGS__))
#define PRINT_30(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_31(__VA_ARGS__))
#define PRINT_31(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_32(__VA_ARGS__))
#define PRINT_32(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_33(__VA_ARGS__))
#define PRINT_33(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_34(__VA_ARGS__))
#define PRINT_34(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_35(__VA_ARGS__))
#define PRINT_35(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_36(__VA_ARGS__))
#define PRINT_36(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_37(__VA_ARGS__))
#define PRINT_37(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_38(__VA_ARGS__))
#define PRINT_38(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_39(__VA_ARGS__))
#define PRINT_39(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_40(__VA_ARGS__))
#define PRINT_40(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_41(__VA_ARGS__))
#define PRINT_41(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_42(__VA_ARGS__))
#define PRINT_42(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_43(__VA_ARGS__))
#define PRINT_43(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_44(__VA_ARGS__))
#define PRINT_44(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_45(__VA_ARGS__))
#define PRINT_45(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_46(__VA_ARGS__))
#define PRINT_46(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_47(__VA_ARGS__))
#define PRINT_47(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_48(__VA_ARGS__))
#define PRINT_48(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_49(__VA_ARGS__))
#define PRINT_49(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_50(__VA_ARGS__))
#define PRINT_50(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_51(__VA_ARGS__))
#define PRINT_51(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_52(__VA_ARGS__))
#define PRINT_52(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_53(__VA_ARGS__))
#define PRINT_53(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_54(__VA_ARGS__))
#define PRINT_54(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_55(__VA_ARGS__))
#define PRINT_55(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_56(__VA_ARGS__))
#define PRINT_56(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_57(__VA_ARGS__))
#define PRINT_57(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_58(__VA_ARGS__))
#define PRINT_58(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_59(__VA_ARGS__))
#define PRINT_59(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_60(__VA_ARGS__))
#define PRINT_60(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_61(__VA_ARGS__))
#define PRINT_61(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_62(__VA_ARGS__))
#define PRINT_62(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_63(__VA_ARGS__))
#define PRINT_63(V, ...) PRINT_ITEM((V)); __VA_OPT__(PRINT_64(__VA_ARGS__))
#define PRINT_64(V, ...) PRINT_TOO_MANY_ITEMS
