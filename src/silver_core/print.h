typedef u8 PRINT_Info; enum {
	FLAG_DEF(PRINT_PersistSpec)
};

typedef struct {
	Sink sink;
	SinkAttr sink_attr;
	FmtSpec spec;
	PRINT_Info info;
} PRINT_State;

typedef struct {
	usize data;
	SinkRes (*fn)(PRINT_State *state, usize data);
} PRINT_Cmd;

SinkRes PRINT_cmd_setspec(PRINT_State *state, usize data) {
	state->spec = (FmtSpec)data;
	state->info &= FLAG_CLEAR(PRINT_PersistSpec);
}

SinkRes PRINT_cmd_persistspec(PRINT_State *state, usize data) {
	state->spec = (FmtSpec)data;
	state->info |= FLAG_DEF(PRINT_PersistSpec);
}

PRINT_Cmd PRINT_spec(FmtSpec spec) {
	return (PRINT_Cmd) {
		.data = (usize)spec,
		.fn = &PRINT_cmd_setspec
	};
}

PRINT_Cmd PRINT_persistspec(FmtSpec spec) {
	return (PRINT_Cmd) {
		.data = (usize)spec,
		.fn = &PRINT_cmd_persistspec
	};
}

PRINT_Cmd const PRINT_clearspec = {
	.data = 0,
	.fn = &PRINT_cmd_setspec
};

SinkRes PRINT_ITEM_PRINT_Cmd(PRINT_State *state, PRINT_Cmd val) {
	return val.fn(state, val.data);
}

#define PRINT_LOCALS_SHORTHAND \
	[[maybe_unused]] auto const SPEC = PRINT_spec; \
	[[maybe_unused]] auto const CLEAR = PRINT_clearspec; \
	[[maybe_unused]] auto const PERSIST = PRINT_persistspec;

#ifndef PRINT_LOCALS
	#define PRINT_LOCALS PRINT_LOCALS_SHORTHAND
#endif

#define PRINT_INTERNAL(...) \
	PRINT_LOCALS \
	__VA_OPT__(PRINT_0(__VA_ARGS__))

#define PRINT_BUF_INTERNAL(buf_size, sink, ...) \
	usize constexpr PRINT_buf_size__ = (buf_size); \
	Sink const PRINT_parent_sink__ = (sink); \
	ubyte PRINT_buf__[PRINT_buf_size__]; \
	BufferedSink PRINT_buf_sink__; \
	BufferedSink_init(&PRINT_buf_sink__, PRINT_parent_sink__, PRINT_buf__, PRINT_buf_size__); \
	Sink const PRINT_sink__ = BufferedSink_asSink(&PRINT_buf_sink__); \
	PRINT_INTERNAL(__VA_ARGS__) \
	PRINT_res__ = BufferedSink_push_(&PRINT_buf_sink__, 0); \

#ifndef PRINT_BUFSIZE
	#define PRINT_BUFSIZE 64
#endif

#define PRINT_RAW(sink, ...) ({ \
	__label__ PRINT_early_exit__; \
	PRINT_State PRINT_state__ = {}; \
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

#define PRINT_BUF(sink, ...) PRINT_BUF_(PRINT_BUFSIZE, sink, __VA_ARGS__)

#ifndef PRINT
	#define PRINT PRINT_BUF
#endif

#define PRINTR PRINT_RAW
#define PRINTA(sink, ...) \
	SinkRes_assert_(PRINT(sink, __VA_ARGS__), __func__, SOURCE_IDENTIFIER " PRINT failed:")

FmtSpec PRINT_consumespec(PRINT_State *state) {
	FmtSpec res = state->spec;
	if (!(state->info & FLAG(PRINT_PersistSpec)))
		state->spec = FmtSpec_null;
	return res;
}

SinkRes PRINT_ITEM_ullong(PRINT_State *state, ullong val) {
	auto const spec = PRINT_consumespec(state);
	return ullong_fmt(val, (IntFmtSpec)spec, state->sink);
}

SinkRes PRINT_ITEM_illong(PRINT_State *state, illong val) {
	auto const spec = PRINT_consumespec(state);
	return illong_fmt(val, (IntFmtSpec)spec, state->sink);
}

SinkRes PRINT_ITEM_ConstPtr(PRINT_State *state, ConstPtr val) {
	auto spec = PRINT_consumespec(state);
	if (!spec)
		spec = FIELD(IntFmt_Base, Hex) | FLAG(IntFmt_Header);
	return ullong_fmt((ullong)val, (IntFmtSpec)spec, state->sink);
}

SinkRes PRINT_ITEM_bool(PRINT_State *state, bool val) {
	PRINT_consumespec(state);

	Str str;
	usize str_size;
	if (val) {
		str = "true";
		str_size = 4;
	} else {
		str = "false";
		str_size = 5;
	}

	return Sink_write(state->sink, str, str_size);
}

SinkRes PRINT_ITEM_Fmt(PRINT_State *state, Fmt val) {
	auto const spec = PRINT_consumespec(state);
	return Fmt_invoke(val, spec, state->sink);
}

SinkRes PRINT_ITEM_Str(PRINT_State *state, Str val) {
	PRINT_consumespec(state);
	return Sink_write(state->sink, val, __builtin_strlen(val));
}

SinkRes PRINT_ITEM_String(PRINT_State *state, String val) {
	PRINT_consumespec(state);
	return Sink_write(state->sink, val.data, val.size);
}

#define PRINT_ITEM(V) \
	PRINT_res__ = _Generic((V), \
		PRINT_Cmd : PRINT_ITEM_PRINT_Cmd, \
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
	)(PRINT_state__, (V)); \
	if (PRINT_res__) goto PRINT_early_exit__; \

#define PRINT_0(V, ...) PRINT_ITEM((V))__VA_OPT__(PRINT_1(__VA_ARGS__))

#define PRINT_ERROR_TOO_MANY_ITEMS _Pragma("GCC error "\"PRINT: too many items\"")

#include "print_gen.h"
