#include "silver_linux/_include.h"
#include "silver/_include.h"
#include "silver_main.h"

#if BUILD_TESTING

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

#else
	int main(int argc, Str *arg, Str *envp) {
		return 0;
	}

#endif

#if BUILD_NOLIBC
	LINUX_START_ASM(128, silver_main);
#endif
