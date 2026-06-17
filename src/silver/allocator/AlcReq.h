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

constexpr u8 Alc_ResolveNone = u8_max;

typedef u8 AlcFlag; enum {
	AlcFlag_BIT_Zero
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

typedef u16 AlcAttr; enum {
	AlcAttr_BIT_IntentNew,
	AlcAttr_BIT_IntentResize,
	AlcAttr_BIT_IntentDelete,
	AlcAttr_BIT_IntentQuery,
	AlcAttr_BIT_IntentResizeQuery,
	AlcAttr_BIT_IntentPromise,
	AlcAttr_BIT_IntentResizePromise,
	AlcAttr_BIT_IntentLock,
	AlcAttr_BIT_IntentUnlock,
	AlcAttr_BIT_FeatureZero,
	AlcAttr_BIT_FeatureRelativeLocal,
	AlcAttr_BIT_FeatureRelativeS16,
	AlcAttr_BIT_FeatureRelativeU16,
	AlcAttr_BIT_FeatureRelativeS32,
	AlcAttr_BIT_FeatureRelativeU32,
	AlcAttr_BIT_ThreadSafe
};
