TEST(ZZ) {
	usize alc_size;
	auto const res = OsAlc_invoke(nullptr,
		AlcReq_new(2048, 8, true)
	, &alc_size, nullptr);

	ubyte *p = AlcPtr_UNWRAP(res);

	PRINTA(Stdout, p," ",alc_size,"\n");

	return 0;
}
