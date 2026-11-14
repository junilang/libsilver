#ifndef SinkSize_WIDTH
	#define SinkSize_WIDTH 48
#endif

#if SIZE_WIDTH < SinkSize_WIDTH
	#undef SinkSize_WIDTH
	#define SinkSize_WIDTH SIZE_WIDTH
#endif

auto constexpr SinkSize_width = SinkSize_WIDTH;

ENUM_GENERATE(SinkIntent, u8,
	Write, Flush, Push, Attr, Lock, Unlock
)

auto constexpr SinkIntent_width = ENUM_WIDTH(SinkIntent);

typedef u64 SinkReq; enum {
	FIELD_DEF(SinkSize, SinkSize_width),
	FIELD_DEF(SinkIntent, SinkIntent_width),
	FLAGS_DEF(SinkFlag,
		Flush, Push, Partial, Locked, NoWait
	),
	SinkReq_width
};

static_assert(SinkReq_width <= 64);

auto constexpr SinkSize_max = FIELD_MAX(SinkSize);

typedef u32 SinkAttr; enum {
	FLAGS_DEF(SinkAttr,
		IntentWrite,
		IntentFlush,
		IntentPush,
		IntentLock,
		IntentUnlock,
		Concurrent,
		Buffered,
		StatefulLock
	),
	SinkAttr_width
};

static_assert(SinkAttr_width <= 32);
