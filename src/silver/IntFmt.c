typedef unsigned long long IntFmt_Unsigned;
typedef signed long long IntFmt_Signed;

enum {
	IntFmt_Base_FIELD,
	IntFmt_Base_END = IntFmt_Base_FIELD + 1,
	IntFmt_BIT_Negative,
	IntFmt_BIT_Capitalize,
	IntFmt_BIT_Header,
};

typedef enum : u8 {
	IntFmt_Base_Dec,
	IntFmt_Base_Bin,
	IntFmt_Base_Hex
} IntFmt_Base;

typedef u32 IntFmt;

constexpr usize IntFmt_Bufsize = 128;

String IntFmt_Unsigned_tostr(IntFmt_Unsigned val, IntFmt fmt, ubyte buf[IntFmt_Bufsize]) {
	auto be = buf + IntFmt_Bufsize;
	auto bp = buf + IntFmt_Bufsize;

	IntFmt_Base base = FIELD_GET(IntFmt_Base, fmt);
	const bool capitalize = fmt & FLAG(IntFmt, Capitalize);
	const bool header = fmt & FLAG(IntFmt, Header);
	const bool negative = fmt & FLAG(IntFmt, Negative);

	switch (base) {
		case IntFmt_Base_Dec: goto base_dec;
		case IntFmt_Base_Bin: goto base_bin;
		case IntFmt_Base_Hex: goto base_hex;
	}

	if (0) base_dec: {
		if (val == 0)
			*(--bp) = '0';
		else while (val > 0) {
			*(--bp) = '0' + (ubyte)(val % 10);
			val /= 10;
		}
	}

	if (0) base_bin: {
		if (val == 0)
			*(--bp) = '0';
		else while (val > 0) {
			--bp;
			if (val & 1)
				*bp = '1';
			else
				*bp = '0';
			val >>= 1;
		}
		if (header) {
			bp -= 2;
			bp[0] = '0';
			bp[1] = 'b';
		}
	}

	if (0) base_hex: {
		if (val == 0)
			*(--bp) = '0';
		else while (val > 0) {
			--bp;
			ubyte c = val & 0b1111;
			if (c < 10)
				*bp = '0' + c;
			else if (capitalize)
				*bp = 'A' + (c - 10);
			else
				*bp = 'a' + (c - 10);

			val >>= 4;
		}
		if (header) {
			bp -= 2;
			bp[0] = '0';
			bp[1] = 'x';
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
