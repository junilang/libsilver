TEST(ZZ) {
	constexpr usize bsize = 32;
	ubyte buf[bsize + 8];
	usize bend = 0;

	constexpr uint num = 5;

	auto const r = ullong_fmt(num, FLAG(IntFmt_ZeroOffset), buf, bsize);
	if (r < 0)
		PANIC_PRINT(r);
	else
		bend += (usize)r;

	buf[bend++] = '\n';

	(void)OutStream_write(Stdout, buf, bend);

	return 0;
}
