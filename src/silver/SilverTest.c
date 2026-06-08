enum {
	SilverTest_OK = 0,
	SilverTest_ERROR = 1,
};

typedef struct {
	int x;
} SilverTestContext;

typedef int (*SilverTest)(SilverTestContext *ctx);

typedef struct {
	uint total;
	uint failed;
} SilverTestState;

void SilverTest_run(SilverTestState *state, String name, SilverTest entry) {
	FPRINTB(128, Stdout, "> ",name,"\n");

	state->total++;

	SilverTestContext ctx;

	int code = entry(&ctx);

	if (code != SilverTest_OK) {
		state->failed++;
		FPRINTB(128, Stdout, "> ",name," failed: ",code,"\n");
	}
}

typedef void (*SilverTestSuite)(SilverTestState *state);

int SilverTest_entry(int argc, Str *argv, SilverTestSuite suite) {
	SilverTestState state = {
		.total = 0,
		.failed = 0
	};

	suite(&state);

	if (state.failed) {
		FPRINTB(128, Stdout, "\n> ",state.failed," tests failed\n");
		return 1;
	}

	return 0;
}
