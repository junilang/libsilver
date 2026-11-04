#if !LIBSILVER_CORE_INCLUDE
	#include "silver_core.h"
#endif

#if !LIBSILVER_TEST_INCLUDE
	#define LIBSILVER_TEST_INCLUDE true

	typedef struct {
		OutStream log;
	} SilverTestContext;

	typedef int (*SilverTest)(SilverTestContext *ctx);

	typedef struct {
		uint total;
		uint failed;
		OutStream log;
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

		OutStreamRes_ASSERT(
			OutStream_flush(state->log)
		);
	}

	typedef void (*SilverTestSuite)(SilverTestState *state);

	#ifndef LIBSILVER_TEST_STREAM
		#define LIBSILVER_TEST_STREAM Stdout
	#endif

	int SilverTest_entry(int argc, Str *argv, SilverTestSuite suite) {
		int res = 0;

		const OutStream pstream = LIBSILVER_TEST_STREAM;

		ubyte buf[1024];
		BufferedOutStream bufstream;
		BufferedOutStream_init(&bufstream, pstream, buf, COUNTOF(buf));


		SilverTestState state = {
			.total = 0,
			.failed = 0,
			.log = BufferedOutStream_upcast(&bufstream)
		};

		suite(&state);

		if (state.failed) {
			PRINTA(state.log, "\n> ",state.failed," tests failed\n");
			res = 1;
		}

		OutStreamRes_ASSERT(BufferedOutStream_flush(&bufstream));
		return res;
	}

#endif
