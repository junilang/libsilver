#ifndef INT_FMT_BUFSIZE
	#define INT_FMT_BUFSIZE 256
#endif

// argument order optimized for PRINT
SinkRes ullong_fmt(ullong val, IntFmtSpec spec, Sink sink) {
	constexpr usize buf_size = INT_FMT_BUFSIZE;
	ubyte buf[buf_size];

	#if LIBSILVER_SAFE
		// we dont want these flags
		if (spec & FLAGS(IntFmt, ZeroOffset, NullTerminate))
			return SinkRes_ErrInvalidSpec;
	#else
		spec &= FLAGS_CLEAR(IntFmt, ZeroOffset, NullTerminate);
	#endif

	auto const res = ullong_rawfmt(val, spec, buf, buf_size);
	if (res < 0) switch (-res) {
		case IntFmtRes_ErrInvalidSpec:
			return SinkRes_ErrInvalidSpec;
		case IntFmtRes_ErrOverflow:
			return SinkRes_ErrOverflow;
		default:
			return SinkRes_ErrUnknown;
	}

	usize const len = (usize)res;
	return Sink_write(sink, buf + (buf_size - len), len);
}

SinkRes illong_fmt(illong val, IntFmtSpec spec, Sink sink) {
	constexpr usize buf_size = INT_FMT_BUFSIZE;
	ubyte buf[buf_size];

	#if LIBSILVER_SAFE
		// we dont want these flags
		if (spec & FLAGS(IntFmt, ZeroOffset, NullTerminate))
			return SinkRes_ErrInvalidSpec;
	#else
		spec &= FLAGS_CLEAR(IntFmt, ZeroOffset, NullTerminate);
	#endif

	auto const res = illong_rawfmt(val, spec, buf, buf_size);
	if (res < 0) switch (-res) {
		case IntFmtRes_ErrInvalidSpec:
			return SinkRes_ErrInvalidSpec;
		case IntFmtRes_ErrOverflow:
			return SinkRes_ErrOverflow;
		default:
			return SinkRes_ErrUnknown;
	}

	usize const len = (usize)res;
	return Sink_write(sink, buf + (buf_size - len), len);
}

SinkRes IFmt_ullong(Ptr this, FmtSpec spec, IARG(Sink, sink)) {
	return ullong_fmt((ullong)this, (IntFmtSpec)spec, IWRAP(Sink, sink));
}

SinkRes IFmt_illong(Ptr this, FmtSpec spec, IARG(Sink, sink)) {
	return illong_fmt((illong)this, (IntFmtSpec)spec, IWRAP(Sink, sink));
}
