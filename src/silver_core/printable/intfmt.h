#ifndef INT_PRINT_BUFSIZE
	#define INT_PRINT_BUFSIZE 256
#endif

OutStreamRes ullong_print(ullong val, PrintFmt vfmt, OutStream os) {
	constexpr usize buf_size = INT_PRINT_BUFSIZE;
	ubyte buf[buf_size];

	auto fmt = (IntFmt)vfmt.value;
	#if LIBSILVER_SAFE
		// we dont want these flags
		if (fmt & FLAGS(IntFmt, ZeroOffset, NullTerminate))
			return OutStreamRes_ErrInvalidInput;
	#else
		fmt &= FLAGS_CLEAR(IntFmt, ZeroOffset, NullTerminate);
	#endif

	auto const res = ullong_fmt(val, fmt, buf, buf_size);
	if (res < 0) switch (-res) {
		case IntFmtRes_ErrInvalidFmt:
			return OutStreamRes_ErrInvalidInput;
		case IntFmtRes_ErrOverflow:
			return OutStreamRes_ErrOverflow;
		default:
			return OutStreamRes_ErrUnknown;
	}

	usize const len = (usize)res;
	return OutStream_write(os, buf + (buf_size - len), len);
}

OutStreamRes illong_print(illong val, PrintFmt vfmt, OutStream os) {
	constexpr usize buf_size = INT_PRINT_BUFSIZE;
	ubyte buf[buf_size];

	auto fmt = (IntFmt)vfmt.value;
	#if LIBSILVER_SAFE
		// we dont want these flags
		if (fmt & FLAGS(IntFmt, ZeroOffset, NullTerminate))
			return OutStreamRes_ErrInvalidInput;
	#else
		fmt &= FLAGS_CLEAR(IntFmt, ZeroOffset, NullTerminate);
	#endif

	auto const res = illong_fmt(val, fmt, buf, buf_size);
	if (res < 0) switch (-res) {
		case IntFmtRes_ErrInvalidFmt:
			return OutStreamRes_ErrInvalidInput;
		case IntFmtRes_ErrOverflow:
			return OutStreamRes_ErrOverflow;
		default:
			return OutStreamRes_ErrUnknown;
	}

	usize const len = (usize)res;
	return OutStream_write(os, buf + (buf_size - len), len);
}

OutStreamRes ullongPtr_print(const ullong *this, PrintFmt fmt, OutStream os) {
	return ullong_print(*this, fmt, os);
}

IPrintable_GENERATE_KNOWN(ullongPtr, const ullong*)

OutStreamRes illongPtr_print(const illong *this, PrintFmt fmt, OutStream os) {
	return illong_print(*this, fmt, os);
}

IPrintable_GENERATE_KNOWN(illongPtr, const illong*)
