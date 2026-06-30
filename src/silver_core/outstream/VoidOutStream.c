OutStreamRes VoidOutStream_write(usize this, ConstPtr buffer, usize buffer_size) {
	if (!this)
		return OutStreamRes_ErrInvalidStream;

	return OutStreamRes_Ok;
}

OutStreamRes VoidOutStream_flush(usize this) {
	if (!this)
		return OutStreamRes_ErrInvalidStream;

	return OutStreamRes_Ok;
}

OutStreamAttr VoidOutStream_attr(usize this) {
	return 0;
}

IOutStream_GENERATE_KNOWN(VoidOutStream, usize)

#if OutStream_PTRTAG
	#define XCONST constexpr
#else
	#define XCONST const
#endif

// generates errors on usage
XCONST OutStream NullOutStream = CONSTEXPR_OutStream_upcast(VoidOutStream, 0);

// silently does nothing
XCONST OutStream VoidOutStream = CONSTEXPR_OutStream_upcast(VoidOutStream, 1);

#undef XCONST
