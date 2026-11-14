SinkRes OsSink_invoke(linux_fd fd, SinkReq req, Ptr aux, ConstPtr src) {
	constexpr SinkAttr attr = FLAGS(SinkAttr,
		IntentWrite, IntentFlush, Concurrent
	);

	switch (FIELD_GET_CAST(SinkIntent, req)) {
		default:
			return SinkRes_ErrUnsupported;

		case SinkIntent_Push:
		case SinkIntent_Flush:
			return SinkRes_Ok;

		case SinkIntent_Attr:
			*(SinkAttr*)aux = attr;
			return SinkRes_Ok;

		case SinkIntent_Write:
	}

	usize const req_size = FIELD_GET(SinkSize, req);
	auto const res = linux_write(fd, src, req_size);
	if (res < 0) switch (-res) {
		default:
			return SinkRes_ErrUnknown;
	}

	auto const write_size = (usize)res;
	if (aux) *(usize*)aux = write_size;

	if (write_size < req_size) {
		if (req & FLAG(SinkFlag_Partial))
			return SinkRes_Partial;
		return SinkRes_ErrOverflow;
	}

	return SinkRes_Ok;
}

ISink_GENERATE_KNOWN(OsSink, linux_fd)

#if OutStream_PTRTAG
	#define XCONST constexpr
#else
	#define XCONST const
#endif

XCONST Sink Stdout = CONSTEXPR_Sink_upcast(OsSink, linux_stdout);
XCONST Sink Stderr = CONSTEXPR_Sink_upcast(OsSink, linux_stderr);

#undef XCONST

Sink os_panic_sink() {
	return Stderr;
}

Sink os_stdout_sink() {
	return Stdout;
}

Sink os_stderr_sink() {
	return Stderr;
}
