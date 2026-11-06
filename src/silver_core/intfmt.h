typedef u32 IntFmt; enum {
	FIELD_DEF(IntFmt_Base, 2),
	FIELD_DEF(IntFmt_Delimiter, 2),
	FIELD_DEF(IntFmt_Digits, 8),
	FIELD_DEF(IntFmt_Spacing, 8),
	FLAG_DEF(IntFmt_FillSpacing),
	FLAG_DEF(IntFmt_Negative),
	FLAG_DEF(IntFmt_Capitalize),
	FLAG_DEF(IntFmt_Header),
	FLAG_DEF(IntFmt_ZeroOffset),
	FLAG_DEF(IntFmt_NullTerminate)
};

enum {
	IntFmt_Delimiter_Space,
	IntFmt_Delimiter_Underscore,
	IntFmt_Delimiter_Dash,
	IntFmt_Delimiter_Quote
};

enum {
	IntFmt_Base_Dec,
	IntFmt_Base_Bin,
	IntFmt_Base_Hex,
	IntFmt_Base_Oct
};

#define XS \
	X(Ok) \
	X(ErrInvalidFmt) \
	X(ErrOverflow) \
	X(ErrUnknown)

	enum {
		#define X(N) IntFmtRes_##N,
			XS
		#undef X
	};

	const String IntFmtRes_repr[] = {
		#define X(N) [IntFmtRes_##N] = String_INIT(#N),
			XS
		#undef X
	};

#undef XS

usize IntFmtRes_put(int r, ubyte *buf, usize buf_size) {
	if (r > 0) r = IntFmtRes_Ok;
	else if (r < -IntFmtRes_ErrUnknown) r = IntFmtRes_ErrUnknown;
	else r = -r;

	String s = IntFmtRes_repr[r];
	if (s.size > buf_size) s.size = buf_size;
	memcpy(buf, s.data, s.size);
	return s.size;
}


// returns length of generated string representation
//	including null terminator (if requested by format specifier)
// by default the string is written from the end of the provided
// output buffer to the front, unless ZeroOffset flag is set
int ullong_fmt(ullong val, IntFmt fmt, ubyte *const buf, usize const buf_size) {
	ubyte *bp = buf + buf_size;

	u8 const base = FIELD_GET(IntFmt_Base, fmt);
	if (base > IntFmt_Base_Oct) {
		return -IntFmtRes_ErrInvalidFmt;
	}

	ubyte delim;
	switch (FIELD_GET(IntFmt_Delimiter, fmt)) {
		case IntFmt_Delimiter_Space:
			delim = ' ';
			break;
		case IntFmt_Delimiter_Underscore:
			delim = '_';
			break;
		case IntFmt_Delimiter_Dash:
			delim = '-';
			break;
		case IntFmt_Delimiter_Quote:
			delim = '\'';
			break;
		default:
			// invalid delimiter value
			return -IntFmtRes_ErrInvalidFmt;
	}

	u16 const digits = FIELD_GET(IntFmt_Digits, fmt);
	u16 dig = 0;

	u16 spacing = FIELD_GET(IntFmt_Spacing, fmt);
	if (!spacing)
		spacing = u16_max;

	auto spc = spacing;

	bool const capitalize = fmt & FLAG(IntFmt_Capitalize);
	bool const header = fmt & FLAG(IntFmt_Header);
	bool const negative = fmt & FLAG(IntFmt_Negative);
	bool const fillspacing = fmt & FLAG(IntFmt_FillSpacing);
	bool const zerooffset = fmt & FLAG(IntFmt_FillSpacing);
	bool const nullterminate = fmt & FLAG(IntFmt_FillSpacing);

	#define XPUT(n_) { \
		auto const n = (n_); \
		auto const d = bp - buf; \
		if (d >= n) bp -= n; \
		else return -IntFmtRes_ErrOverflow; \
	}

	if (nullterminate) {
		XPUT(1);
		bp[0] = 0;
	}

	if (!val) {
		XPUT(1);
		bp[0] = '0';
		spc--;
		dig++;
		goto skip_body;
	}

	switch (base) {
		case IntFmt_Base_Dec: goto base_dec;
		case IntFmt_Base_Bin: goto base_bin;
		case IntFmt_Base_Hex: goto base_hex;
		case IntFmt_Base_Oct: goto base_oct;
	}

	// value of base was validated before
	return -IntFmtRes_ErrUnknown;


	if (0) base_dec: do {
		ubyte const c = '0' + val % 10;
		val /= 10;
		dig++;
		if (spc) {
			XPUT(1);
			bp[0] = c;
			spc--;
		} else {
			XPUT(2);
			bp[0] = c;
			bp[1] = delim;
			spc = spacing - 1;
		}
	} while (val);

	if (0) base_bin: do {
		ubyte const c = '0' + (val & 1);
		val >>= 1;
		dig++;
		if (spc) {
			XPUT(1);
			bp[0] = c;
			spc--;
		} else {
			XPUT(2);
			bp[0] = c;
			bp[1] = delim;
			spc = spacing - 1;
		}
	} while (val);

	if (0) base_hex: do {
		ubyte c = val & 0b1111;
		if (c < 10) c += '0';
		else if (capitalize) c += ('A' - 10);
		else c += ('a' - 10);
		val >>= 4;
		dig++;
		if (spc) {
			XPUT(1);
			bp[0] = c;
			spc--;
		} else {
			XPUT(2);
			bp[0] = c;
			bp[1] = delim;
			spc = spacing - 1;
		}
	} while (val);

	if (0) base_oct: do {
		ubyte const c = '0' + (val & 0b111);
		val >>= 3;
		dig++;
		if (spc) {
			XPUT(1);
			bp[0] = c;
			spc--;
		} else {
			XPUT(2);
			bp[0] = c;
			bp[1] = delim;
			spc = spacing - 1;
		}
	} while (val);

	skip_body:;

	while (dig < digits) {
		dig++;
		if (spc) {
			XPUT(1);
			bp[0] = '0';
			spc--;
		} else {
			XPUT(2);
			bp[0] = '0';
			bp[1] = delim;
			spc = spacing - 1;
		}
	}

	if (fillspacing && spc) {
		XPUT(spc);
		do {
			bp[--spc] = '0';
		} while (spc);
	}

	if (header) {
		constexpr ubyte basechars[] = {'d', 'b', 'x', 'o'};

		XPUT(2);
		bp[0] = '0';
		bp[1] = basechars[base];
	}

	if (negative) {
		XPUT(1);
		bp[0] = '-';
	}

	usize const len = buf_size - (usize)(bp - buf);

	// move output string to beginning of buffer
	if (zerooffset && bp > buf) {
		for (usize i = 0; i < len; i++) {
			buf[i] = bp[i];
		}
	}

	if (len > int_max)
		return -IntFmtRes_ErrOverflow;

	return (int)len;

	#undef XPUT
}

int illong_fmt(illong val, IntFmt fmt, ubyte *buf, usize buf_size) {
	ullong uval;
	if (val == illong_min) {
		fmt |= FLAG(IntFmt_Negative);
		uval = (ullong)illong_max + 1;
	} else if (val < 0) {
		fmt |= FLAG(IntFmt_Negative);
		uval = (ullong)(-val);
	} else {
		uval = (ullong)val;
	}

	return ullong_fmt(uval, fmt, buf, buf_size);
}
