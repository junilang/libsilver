typedef unsigned long long IntFmt_Unsigned;
typedef signed long long IntFmt_Signed;

enum {
	FIELD_DEF(IntFmt_Base, 2),
	FIELD_DEF(IntFmt_Spacing, 5),
	FIELD_DEF(IntFmt_Delimiter, 2),
	FIELD_DEF(IntFmt_Digits, 8), // up to 255 digits
	FLAG_DEF(IntFmt_Negative),
	FLAG_DEF(IntFmt_Capitalize),
	FLAG_DEF(IntFmt_Header),
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

	auto const base = FIELD_GET_CAST(IntFmt_Base, fmt);
	const bool capitalize = fmt & FLAG(IntFmt_Capitalize);
	const bool header = fmt & FLAG(IntFmt_Header);
	const bool negative = fmt & FLAG(IntFmt_Negative);

	const u8 digits = FIELD_GET(IntFmt_Digits, fmt);

	u8 delimiter;
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
		IntFmt_Unsigned_tostr(val, fmt.value, buf),
		PrintFmt_Null, os
	);
}

OutStreamRes IntFmt_Signed_print(IntFmt_Signed val, PrintFmt fmt, OutStream os) {
	ubyte buf[IntFmt_Bufsize];
	return String_print(
		IntFmt_Signed_tostr(val, fmt.value, buf),
		PrintFmt_Null, os
	);
}
