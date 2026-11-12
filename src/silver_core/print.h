#define PRINT_INTERNAL(...) \
	[[maybe_unused]] auto const SPEC = &PRINT_FmtSpec_create; \
	[[maybe_unused]] auto const S = &PRINT_FmtSpec_create; \
	[[maybe_unused]] PRINT_CmdFlush constexpr FLUSH = {}; \
	[[maybe_unused]] PRINT_CmdPush constexpr PUSH = {}; \
	FmtSpec PRINT_spec__ = FmtSpec_null; \
	__VA_OPT__(PRINT_0(__VA_ARGS__))

#define PRINT_BUF_INTERNAL(buf_size, sink, ...) \
	usize constexpr PRINT_buf_size__ = (buf_size); \
	Sink const PRINT_parent_sink__ = (sink); \
	ubyte PRINT_buf__[PRINT_buf_size__]; \
	BufferedSink PRINT_buf_sink__; \
	BufferedSink_init(&PRINT_buf_sink__, PRINT_parent_sink__, PRINT_buf__, PRINT_buf_size__); \
	Sink const PRINT_sink__ = BufferedSink_asSink(&PRINT_buf_sink__); \
	PRINT_INTERNAL(__VA_ARGS__) \
	PRINT_res__ = BufferedSink_push(&PRINT_buf_sink__); \

#ifndef PRINT_BUF_SIZE
	#define PRINT_BUF_SIZE 64
#endif

#define PRINT_RAW(sink, ...) ({ \
	__label__ PRINT_early_exit__; \
	SinkRes PRINT_res__ = SinkRes_Ok; \
	Sink const PRINT_sink__ = (sink); \
	PRINT_INTERNAL(__VA_ARGS__) \
	PRINT_early_exit__:; \
	PRINT_res__; \
})

#define PRINT_BUF_(buf_size, sink, ...) ({ \
	__label__ PRINT_early_exit__; \
	SinkRes PRINT_res__ = SinkRes_Ok; \
	PRINT_BUF_INTERNAL(buf_size, sink, __VA_ARGS__) \
	PRINT_early_exit__:; \
	PRINT_res__; \
})

#define PRINT_BUF(sink, ...) PRINT_BUF_(PRINT_BUF_SIZE, sink, __VA_ARGS__)

#ifndef PRINT
	#define PRINT PRINT_BUF
#endif

#define PRINTR PRINT_RAW
#define PRINTA(sink, ...) \
	SinkRes_assert_(PRINT(sink, __VA_ARGS__), __func__, SOURCE_IDENTIFIER " PRINT failed:")

typedef struct {
	FmtSpec value;
} PRINT_FmtSpec;

PRINT_FmtSpec PRINT_FmtSpec_create(FmtSpec value) {
	return (PRINT_FmtSpec){value};
}

SinkRes PRINT_ITEM_PRINT_FmtSpec(Sink sink, FmtSpec *specp, PRINT_FmtSpec val) {
	*specp = val.value;
	return SinkRes_Ok;
}

typedef struct { ubyte dummy__; } PRINT_CmdFlush;

SinkRes PRINT_ITEM_PRINT_CmdFlush(Sink sink, FmtSpec *specp, PRINT_CmdFlush val) {
	return Sink_flush(sink);
}

typedef struct { ubyte dummy__; } PRINT_CmdPush;

SinkRes PRINT_ITEM_PRINT_CmdPush(Sink sink, FmtSpec *specp, PRINT_CmdPush val) {
	return Sink_push(sink);
}

SinkRes PRINT_ITEM_illong(Sink sink, FmtSpec *specp, illong val) {
	auto const spec = *specp;
	*specp = FmtSpec_null;
	return illong_fmt(val, (IntFmtSpec)spec, sink);
}

SinkRes PRINT_ITEM_ullong(Sink sink, FmtSpec *specp, ullong val) {
	auto const spec = *specp;
	*specp = FmtSpec_null;
	return ullong_fmt(val, (IntFmtSpec)spec, sink);
}

SinkRes PRINT_ITEM_ConstPtr(Sink sink, FmtSpec *specp, ConstPtr val) {
	auto spec = *specp;
	*specp = FmtSpec_null;
	if (!spec)
		spec = FIELD(IntFmt_Base, Hex) | FLAG(IntFmt_Header);
	return ullong_fmt((ullong)val, (IntFmtSpec)spec, sink);
}

SinkRes PRINT_ITEM_bool(Sink sink, FmtSpec *specp, bool val) {
	*specp = FmtSpec_null;

	Str str;
	usize str_size;
	if (val) {
		str = "true";
		str_size = 4;
	} else {
		str = "false";
		str_size = 5;
	}

	return Sink_write(sink, str, str_size);
}

SinkRes PRINT_ITEM_Fmt(Sink sink, FmtSpec *specp, Fmt val) {
	auto const spec = *specp;
	*specp = FmtSpec_null;
	return Fmt_invoke(val, spec, sink);
}

SinkRes PRINT_ITEM_Str(Sink sink, FmtSpec *specp, Str val) {
	*specp = FmtSpec_null;
	return Sink_write(sink, val, __builtin_strlen(val));
}

SinkRes PRINT_ITEM_String(Sink sink, FmtSpec *specp, String val) {
	*specp = FmtSpec_null;
	return Sink_write(sink, val.data, val.size);
}

#define PRINT_ITEM(V) { \
	PRINT_res__ = _Generic((V), \
		PRINT_FmtSpec : PRINT_ITEM_PRINT_FmtSpec, \
		PRINT_CmdFlush : PRINT_ITEM_PRINT_CmdFlush, \
		PRINT_CmdPush : PRINT_ITEM_PRINT_CmdPush, \
		char * : PRINT_ITEM_Str, \
		const char * : PRINT_ITEM_Str, \
		String : PRINT_ITEM_String, \
		Fmt : PRINT_ITEM_Fmt, \
		bool : PRINT_ITEM_bool, \
		ichar : PRINT_ITEM_illong, \
		ishort : PRINT_ITEM_illong, \
		int : PRINT_ITEM_illong, \
		ilong : PRINT_ITEM_illong, \
		illong : PRINT_ITEM_illong, \
		uchar : PRINT_ITEM_ullong, \
		ushort : PRINT_ITEM_ullong, \
		uint : PRINT_ITEM_ullong, \
		ulong : PRINT_ITEM_ullong, \
		ullong : PRINT_ITEM_ullong, \
		default: PRINT_ITEM_ConstPtr \
	)(PRINT_sink__, &PRINT_spec__, (V)); \
	if (PRINT_res__) goto PRINT_early_exit__; \
}

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
#define PRINT_64(V, ...) _Pragma("GCC error "\"PRINT: too many items (max 64)\"")
