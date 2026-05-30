#define RioTask_ALIGNMENT alignof(usize)

typedef enum : u8 {
	RioTaskType_NOP
} RioTaskType;

typedef u8 RioTaskMetadata;

typedef struct {
	Ptr value;
} RioTask;
