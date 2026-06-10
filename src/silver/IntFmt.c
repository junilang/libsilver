typedef unsigned long long IntFmt_Unsigned;
typedef signed long long IntFmt_Signed;

enum {
	IntFmt_Base_FIELD,
	IntFmt_Base_END = IntFmt_Base_FIELD + 1,
	IntFmt_Spacing_FIELD,
	IntFmt_Spacing_END = IntFmt_Spacing_FIELD + 4,
	IntFmt_Delimiter_FIELD,
	IntFmt_Delimiter_END = IntFmt_Delimiter_FIELD + 2,
	IntFmt_BIT_Negative,
	IntFmt_BIT_Capitalize,
	IntFmt_BIT_Header,
};

typedef enum : u8 {
	IntFmt_Delimiter_Space,
	IntFmt_Delimiter_Underscore,
	IntFmt_Delimiter_Dash
} IntFmt_Delimiter;

typedef enum : u8 {
	IntFmt_Base_Dec,
	IntFmt_Base_Bin,
	IntFmt_Base_Hex,
	IntFmt_Base_Oct
} IntFmt_Base;

typedef u32 IntFmt;

constexpr usize IntFmt_Bufsize = 128;

String IntFmt_Unsigned_tostr(IntFmt_Unsigned val, IntFmt fmt, ubyte buf[IntFmt_Bufsize]) {
	auto be = buf + IntFmt_Bufsize;
	auto bp = buf + IntFmt_Bufsize;

	auto const base = FIELD_CGET(IntFmt_Base, fmt);
	const bool capitalize = fmt & FLAG(IntFmt, Capitalize);
	const bool header = fmt & FLAG(IntFmt, Header);
	const bool negative = fmt & FLAG(IntFmt, Negative);

	u8 delimiter;
	u8 spacing = FIELD_GET(IntFmt_Spacing, fmt);
	if (!spacing)
		spacing = u8_max;
	else {
		switch (FIELD_CGET(IntFmt_Delimiter, fmt)) {
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
	u8 wid = 0;

	if (val == 0) {
		*(--bp) = '0';
		spc--;
		wid++;
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
			wid++;
		}
		*(--bp) = '0' + (ubyte)(val % 10);
		val /= 10;
		spc--;
		wid++;
	}

	if (0) base_bin: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
			wid++;
		}
		--bp;
		if (val & 1)
			*bp = '1';
		else
			*bp = '0';
		val >>= 1;
		spc--;
		wid++;
	}

	if (0) base_hex: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
			wid++;
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
		wid++;
	}

	if (0) base_oct: while (val > 0) {
		if (spc == 0) {
			*(--bp) = delimiter;
			spc = spacing;
			wid++;
		}
		--bp;
		ubyte c = val & 0b111;
		*bp = '0' + c;
		val >>= 3;
		spc--;
		wid++;
	}

	skip_body:;

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
		fmt |= FLAG(IntFmt, Negative);
		val = -val;
	}
	return IntFmt_Unsigned_tostr((IntFmt_Unsigned)val, fmt, buf);
}


void IntFmt_Unsigned_print(IntFmt_Unsigned val, PrintFmt fmt, OutStream os) {
	ubyte buf[IntFmt_Bufsize];
	String_print(
		IntFmt_Unsigned_tostr(val, fmt.value, buf),
		PrintFmt_Null, os
	);
}

void IntFmt_Signed_print(IntFmt_Signed val, PrintFmt fmt, OutStream os) {
	ubyte buf[IntFmt_Bufsize];
	String_print(
		IntFmt_Signed_tostr(val, fmt.value, buf),
		PrintFmt_Null, os
	);
}
