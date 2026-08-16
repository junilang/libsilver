#define XS \
	X(EINVAL) \
	X(ENOENT)

const char *linux_error_map[] = {
	[0] = "OK",
	#define X(N) [N] = #N,
		XS
	#undef X
};

#undef XS

constexpr uword linux_error_map_size = COUNTOF(linux_error_map);

const char *linux_error_string(iword result) {
	if (result < 0) result = -result;
	if ((uword)result >= linux_error_map_size) return "EUNKNOWN";

	const char *repr = linux_error_map[result];
	if (!repr) return "EUNKNOWN";

	return repr;
}
