#include "silver_os.h"

#if BUILD_TESTING
	#include "silver_test.h"

	#define TEST_SUITE_BEGIN void libsilver_testsuite(SilverTestState *state) {
	#define TEST_RUN(test, name) SilverTest_run(state, STRING(name), &test##_entry);
	#define TEST_SUITE_END }

	#ifdef BUILD_TESTING_INCLUDE
		#include BUILD_TESTING_INCLUDE
	#else
		TEST_SUITE_BEGIN TEST_SUITE_END
	#endif

	int main(int argc, Str *argv, Str *envp) {
		return SilverTest_entry(argc, argv, &libsilver_testsuite);
	}

#endif
