int ZZentry(SilverTestContext *ctx) {
	int x;
	if (chkdadd(int_max, 1, &x)) {
		PRINT(Stdout, "overflow!\n");
	}

	return 0;
}
