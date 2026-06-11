typedef enum : u8 {
	AlcNrq_Intent_Least,
	AlcNrq_Intent_Range,
	AlcNrq_Intent_Loose,
	AlcNrq_Intent_MAX
} AlcNrq_Intent;

typedef u64 AlcNrq; enum {
	AlcNrq_CommonData_END = AlcReq_CommonData_END,
	FIELD_DEFINE(AlcNrq_Intent, 2),
	FIELD_DEFINE(AlcNrq_Alts, 2)
};

static_assert(FIELD_MAX(AlcNrq_Intent) >= AlcNrq_Intent_MAX - 1);

typedef enum : u8 {
	AlcNrs_Offer_Accept,
	AlcNrs_Offer_Counter,
	AlcNrs_Offer_Refuse,
	AlcNrs_Offer_MAX
} AlcNrs_Offer;

typedef u64 AlcNrs; enum {
	AlcNrs_Size_END = AlcSize_END,
	FIELD_DEFINE(AlcNrs_Offer, 2),
	FIELD_DEFINE(AlcNrs_RefuseReason, 3),
	FIELD_DEFINE(AlcNrs_Alts, 2),
};

static_assert(FIELD_MAX(AlcNrs_RefuseReason) >= AlcRes_MAX - 1);
static_assert(FIELD_MAX(AlcNrs_Offer) >= AlcNrs_Offer_MAX - 1);
