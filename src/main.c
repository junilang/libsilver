#include "libsilver/_include.h"

#if BUILD_TESTING

	#define TEST_SUITE_BEGIN void libsilver_testsuite(SilverTestState *state) {
	#define TEST_RUN(test, name) SilverTest_run(state, STRING(name), &test##_entry);
	#define TEST_SUITE_END }

	#ifdef BUILD_TESTING_INCLUDE
		#include BUILD_TESTING_INCLUDE
	#else
		TEST_SUITE_BEGIN TEST_SUITE_END
	#endif

	int main(int argc, char **argv) {
		return SilverTest_entry(argc, argv, &libsilver_testsuite);
	}
#endif
