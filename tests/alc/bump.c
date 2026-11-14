TEST(ZZ) {
	_Alignas(usize) ubyte buf[1024];
	BumpAlc bumpalc;
	BumpAlc_init(&bumpalc, buf, sizeof(buf));
	auto const alc = BumpAlc_asAlc(&bumpalc);

	usize alc_size;
	auto const restrict res = Alc_invoke(alc,
		AlcReq_new(sizeof(usize), _Alignof(usize), true)
		, &alc_size, nullptr
	);

	usize *p = AlcPtr_UNWRAP(res);

	PRINTA(Stdout,p," ",alc_size,"\n");

	return 0;
}
