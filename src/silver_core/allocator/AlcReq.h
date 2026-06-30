typedef enum : u8 {
	AlcRelative_None,
	AlcRelative_Local,
	AlcRelative_S16,
	AlcRelative_U16,
	AlcRelative_S32,
	AlcRelative_U32,
} AlcRelative;

typedef enum : u8 {
	AlcRange_Least,
	AlcRange_Exact,
	AlcRange_Range,
	AlcRange_Loose,
} AlcRange;

typedef enum : u8 {
	AlcIntent_Attr,
	AlcIntent_New,
	AlcIntent_Resize,
	AlcIntent_Delete,
	AlcIntent_Query,
	AlcIntent_ResizeQuery,
	AlcIntent_Promise,
	AlcIntent_ResizePromise,
	AlcIntent_Lock,
	AlcIntent_Unlock,
} AlcIntent;

constexpr auto Alc_ResolveNone = u8_max;

typedef u8 AlcFlag; enum {
	FLAG_DEF(AlcFlag_Zero)
};

typedef struct {
	usize size;
	ConstPtr hint;
	ualign align;
	AlcIntent intent;
	AlcRelative relative;
	AlcRange range;
	AlcFlag flags;
	u8 offers_size;
} AlcReq;

typedef u32 AlcAttr; enum {
	FLAG_DEF(AlcAttr_IntentNew),
	FLAG_DEF(AlcAttr_IntentResize),
	FLAG_DEF(AlcAttr_IntentDelete),
	FLAG_DEF(AlcAttr_IntentQuery),
	FLAG_DEF(AlcAttr_IntentResizeQuery),
	FLAG_DEF(AlcAttr_IntentPromise),
	FLAG_DEF(AlcAttr_IntentResizePromise),
	FLAG_DEF(AlcAttr_IntentLock),
	FLAG_DEF(AlcAttr_IntentUnlock),
	FLAG_DEF(AlcAttr_FeatureZero),
	FLAG_DEF(AlcAttr_FeatureRelativeLocal),
	FLAG_DEF(AlcAttr_FeatureRelativeS16),
	FLAG_DEF(AlcAttr_FeatureRelativeU16),
	FLAG_DEF(AlcAttr_FeatureRelativeS32),
	FLAG_DEF(AlcAttr_FeatureRelativeU32),
	FLAG_DEF(AlcAttr_ThreadSafe)
};
