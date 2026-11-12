#ifndef SinkSize_WIDTH
	#define SinkSize_WIDTH 48
#endif

#if SIZE_WIDTH < SinkSize_WIDTH
	#undef SinkSize_WIDTH
	#define SinkSize_WIDTH SIZE_WIDTH
#endif

auto constexpr SinkSize_width = SinkSize_WIDTH;

typedef enum : u8 {
	SinkIntent_Write,
	SinkIntent_Flush,
	SinkIntent_Push,
	SinkIntent_Attr,
	SinkIntent_Lock,
	SinkIntent_TryLock,
	SinkIntent_Unlock,
} SinkIntent;

auto constexpr SinkIntent_width = 3;

typedef u64 SinkReq; enum {
	FIELD_DEF(SinkSize, SinkSize_width),
	FIELD_DEF(SinkIntent, SinkIntent_width),
	FLAG_DEF(SinkFlag_Flush),
	FLAG_DEF(SinkFlag_Push),
	FLAG_DEF(SinkFlag_Partial),
	FLAG_DEF(SinkFlag_Locked),
	FLAG_DEF(SinkFlag_NoWait),
	SinkReq_width
};

static_assert(SinkReq_width <= 64);

auto constexpr SinkSize_max = FIELD_MAX(SinkSize);

typedef u32 SinkAttr; enum {
	FLAG_DEF(SinkAttr_IntentWrite),
	FLAG_DEF(SinkAttr_IntentFlush),
	FLAG_DEF(SinkAttr_IntentPush),
	FLAG_DEF(SinkAttr_IntentLock),
	FLAG_DEF(SinkAttr_IntentTryLock),
	FLAG_DEF(SinkAttr_IntentUnlock),
	FLAG_DEF(SinkAttr_ThreadSafe),
};
