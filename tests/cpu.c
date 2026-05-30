int ZZentry(SilverTestContext *ctx) {
	while (true) {
		sched_yield();
	}

	return SilverTest_OK;
}
