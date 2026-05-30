typedef struct {
	RioTaskMetadata meta;
	AsyncFuture future;
} RioTask_Nop;

void RioTask_Nop_submit(Rio *this, RioTask_Nop *task) {

}
