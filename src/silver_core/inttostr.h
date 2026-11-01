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

enum {
	// returned as negatives
	IntToStrErr_InvalidFmt = 1,
	IntToStrErr_Overflow,
	IntToStrErr_Unknown
};

int ullong_tostr(ullong val, IntFmt fmt, ubyte *const buf, usize buf_size) {
	ubyte *bp = buf + buf_size;

	u8 const base = FIELD_GET(IntFmt_Base, fmt);
	if (base > IntFmt_Base_Oct) {
		return -IntToStrErr_InvalidFmt;
	}

	u8 delimiter;
	switch (FIELD_GET(IntFmt_Delimiter, fmt)) {
		case IntFmt_Delimiter_Space:
			delimiter = ' ';
			break;
		case IntFmt_Delimiter_Underscore:
			delimiter = '_';
			break;
		case IntFmt_Delimiter_Dash:
			delimiter = '-';
			break;
		case IntFmt_Delimiter_Quote:
			delimiter = '\'';
			break;
		default:
			// invalid delimiter value
			return -IntToStrErr_InvalidFmt;
	}

	u16 const digits = FIELD_GET(IntFmt_Digits, fmt);
	u16 spacing = FIELD_GET(IntFmt_Spacing, fmt);
	if (!spacing)
		spacing = u16_max;

	bool const capitalize = fmt & FLAG(IntFmt_Capitalize);
	bool const header = fmt & FLAG(IntFmt_Header);
	bool const negative = fmt & FLAG(IntFmt_Negative);
	bool const fillspacing = fmt & FLAG(IntFmt_FillSpacing);
	bool const zerooffset = fmt & FLAG(IntFmt_FillSpacing);
	bool const nullterminate = fmt & FLAG(IntFmt_FillSpacing);


	u16 spc = spacing;
	u16 dig = 0;

	#define XPUT(n_) { \
		auto const n = (n_); \
		auto const d = bp - buf; \
		if (d >= n) bp -= n; \
		else return -IntToStrErr_Overflow; \
	}

	if (nullterminate && !zerooffset) {
		XPUT(1);
		bp[0] = 0;
		buf_size--;
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
	__builtin_unreachable();

	if (0) base_dec: while (val > 0) {
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
			bp[1] = delimiter;
			spc = spacing - 1;
		}
	}

	if (0) base_bin: while (val > 0) {
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
			bp[1] = delimiter;
			spc = spacing - 1;
		}
	}

	if (0) base_hex: while (val > 0) {
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
			bp[1] = delimiter;
			spc = spacing - 1;
		}
	}

	if (0) base_oct: while (val > 0) {
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
			bp[1] = delimiter;
			spc = spacing - 1;
		}
	}

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
			bp[1] = delimiter;
			spc = spacing;
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

	if (zerooffset && bp > buf) {
		
	}

	#undef XPUT
}

int illong_tostr(illong val, IntFmt fmt, ubyte *const buf, usize buf_size) {
	if (val < 0) {
		fmt |= FLAG(IntFmt_Negative);
		val = -val;
	}
	return ullong_tostr((ullong)val, fmt, out_buf, buf_size);
}

#ifndef IntFmt_BUFSIZE
	#define IntFmt_BUFSIZE 128
#endif

constexpr usize IntFmt_bufsize = IntFmt_BUFSIZE;

String IntFmt_Unsigned_tostr(IntFmt_Unsigned val, IntFmt fmt, ubyte *const buf) {
	auto const be = buf + IntFmt_bufsize;
	auto bp = be;

	auto const base = FIELD_GET_CAST(IntFmt_Base, fmt);
	const bool capitalize = fmt & FLAG(IntFmt_Capitalize);
	const bool header = fmt & FLAG(IntFmt_Header);
	const bool negative = fmt & FLAG(IntFmt_Negative);
	const bool fill_spacing = fmt & FLAG(IntFmt_FillSpacing);


	const u8 digits = FIELD_GET(IntFmt_Digits, fmt);

	u8 delimiter = '?';
	u8 spacing = FIELD_GET(IntFmt_Spacing, fmt);
	if (!spacing)
		spacing = u8_max;
	else {
		switch (FIELD_GET_CAST(IntFmt_Delimiter, fmt)) {
			case IntFmt_Delimiter_Space:
				delimiter = ' ';
				break;
			case IntFmt_Delimiter_Underscore:
				delimiter = '_';
				break;
			case IntFmt_Delimiter_Dash:
				delimiter = '-';
				break;
		}
	}

	u8 spc = spacing;
	u8 dig = 0;

	if (val == 0) {
		*(--bp) = '0';
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

	if (0) base_dec: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
		}
		*(--bp) = '0' + (ubyte)(val % 10);
		val /= 10;
		spc--;
		dig++;
	}

	if (0) base_bin: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
		}
		--bp;
		if (val & 1)
			*bp = '1';
		else
			*bp = '0';
		val >>= 1;
		spc--;
		dig++;
	}

	if (0) base_hex: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
		}
		--bp;
		ubyte c = val & 0b1111;
		if (c < 10)
			*bp = '0' + c;
		else if (capitalize)
			*bp = 'A' + (c - 10);
		else
			*bp = 'a' + (c - 10);
		val >>= 4;
		spc--;
		dig++;
	}

	if (0) base_oct: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
		}
		--bp;
		ubyte c = val & 0b111;
		*bp = '0' + c;
		val >>= 3;
		spc--;
		dig++;
	}

	skip_body:;

	while (dig < digits) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
		}
		*(--bp) = '0';
		spc--;
		dig++;
	}

	if (fill_spacing && spc) {
		do {
			*(--bp) = '0';
			spc--;
		} while (spc);
	}

	if (header) {
		bp -= 2;
		switch (base) {
			case IntFmt_Base_Dec:
				bp[0] = '0'; bp[1] = 'd';
				break;
			case IntFmt_Base_Bin:
				bp[0] = '0'; bp[1] = 'b';
				break;
			case IntFmt_Base_Hex:
				bp[0] = '0'; bp[1] = 'x';
				break;
			case IntFmt_Base_Oct:
				bp[0] = '0'; bp[1] = 'o';
				break;
		}
	}

	if (negative) {
		*(--bp) = '-';
	}

	return (String){.data=bp, .size=(usize)(be - bp)};
}

String IntFmt_Signed_tostr(IntFmt_Signed val, IntFmt fmt, ubyte buf[IntFmt_Bufsize]) {
	if (val < 0) {
		fmt |= FLAG(IntFmt_Negative);
		val = -val;
	}
	return IntFmt_Unsigned_tostr((IntFmt_Unsigned)val, fmt, buf);
}


OutStreamRes IntFmt_Unsigned_print(IntFmt_Unsigned val, PrintFmt fmt, OutStream os) {
	ubyte buf[IntFmt_Bufsize];
	return String_print(
		IntFmt_Unsigned_tostr(val, (IntFmt)fmt.value, buf),
		PrintFmt_Null, os
	);
}

OutStreamRes IntFmt_Signed_print(IntFmt_Signed val, PrintFmt fmt, OutStream os) {
	ubyte buf[IntFmt_Bufsize];
	return String_print(
		IntFmt_Signed_tostr(val, (IntFmt)fmt.value, buf),
		PrintFmt_Null, os
	);
}

OutStreamRes IntFmt_UnsignedPtr_print(IntFmt_Unsigned *this, PrintFmt fmt, OutStream os) {
	return IntFmt_Unsigned_print(*this, fmt, os);
}

IPrintable_GENERATE_KNOWN(IntFmt_UnsignedPtr, IntFmt_Unsigned*)

OutStreamRes IntFmt_SignedPtr_print(IntFmt_Signed *this, PrintFmt fmt, OutStream os) {
	return IntFmt_Signed_print(*this, fmt, os);
}

IPrintable_GENERATE_KNOWN(IntFmt_SignedPtr, IntFmt_Signed*)
