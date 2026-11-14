#if !LIBSILVER_CORE_INCLUDE
	#include "silver_core.h"
#endif

#if !LIBSILVER_TEST_INCLUDE
	#define LIBSILVER_TEST_INCLUDE true

	typedef struct {
		Sink log;
	} SilverTestContext;

	typedef int (*SilverTest)(SilverTestContext *ctx);

	typedef struct {
		uint total;
		uint failed;
		Sink log;
	} SilverTestState;

	void SilverTest_run(SilverTestState *state, String name, SilverTest entry) {
		PRINTA(state->log, "> ",name,"\n");

		state->total++;

		SilverTestContext ctx = {
			.log = Stdout
		};

		int code = entry(&ctx);

		if (code != 0) {
			state->failed++;
			PRINTA(state->log, "> ",name," failed: ",code,"\n");
		}

		SinkRes_ASSERT(Sink_flush(state->log));
	}

	typedef void (*SilverTestSuite)(SilverTestState *state);

	int SilverTest_entry(int argc, Str *argv, SilverTestSuite suite) {
		int res = 0;

		SilverTestState state = {
			.total = 0,
			.failed = 0,
			.log = Stdout
		};

		suite(&state);

		if (state.failed) {
			PRINTA(state.log, "\n> ",state.failed," tests failed\n");
			res = 1;
		}

		SinkRes_ASSERT(Sink_flush(state.log));
		return res;
	}

#endif
