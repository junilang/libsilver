#include "silver_os.h"
#include "silver.h"

#if BUILD_NOLIBC
	#include "silver_nolibc.h"
#endif

#if BUILD_TESTING
	#include "silver_test.h"

	#define TEST_SUITE_BEGIN void libsilver_testsuite(SilverTestState *state) {
	#define TEST_RUN(test, name) SilverTest_run(state, STRING(name), (SilverTest)&test##_entry);
	#define TEST_SUITE_END }

	#ifdef BUILD_TESTING_INCLUDE
		#include BUILD_TESTING_INCLUDE
	#else
		TEST_SUITE_BEGIN TEST_SUITE_END
	#endif

	int main(int argc, Str *argv, Str *envp) {
		load_envinfo(argc, argv);
		return SilverTest_entry(argc, argv, &libsilver_testsuite);
	}

#endif
