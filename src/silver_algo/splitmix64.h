u64 splitmix64(u64 *state) {
	auto z = *state + 0x9e3779b97f4a7c15ull;
	*state = z;

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}
