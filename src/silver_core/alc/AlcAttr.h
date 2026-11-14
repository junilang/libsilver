typedef u32 AlcAttr; enum {
	FLAGS_DEF(AlcAttr,
		IntentNew,
		IntentResize,
		IntentDelete,
		IntentLock,
		IntentUnlock,
		FeatureZero,
		Concurrent,
		StatefulLock
	),
	AlcAttr_width
};

static_assert(AlcAttr_width <= 32);
