// Verbose string - used for analyzing utf8 strings

typedef u8 VStringFmt; enum {
	FLAG_DEF(VStringFmt_Code), // print all characters as codes
	FLAG_DEF(VStringFmt_NoUTF), // disable utf processing
	FLAG_DEF(VStringFmt_KeepUTF), // print utf characters as is
	FLAG_DEF(VStringFmt_KeepCTL), // print control characters as is
	FLAG_DEF(VStringFmt_KeepHT), // print horizontal tab
	FLAG_DEF(VStringFmt_KeepLF), // print line feed
	FLAG_DEF(VStringFmt_KeepCR), // print carriage return
};

typedef struct {
	String value;
} VString;

typedef struct {
	SmallString value;
} VSmallString;

OutStreamRes VString_ZZprint(
	OutStream os,
	const ubyte *data, const usize data_size,
	VStringFmt fmt
) {
	const ubyte *const data_end = data + data_size;

	ubyte buffer[128];
	const ubyte *const buffer_end = buffer + sizeof(buffer);
	ubyte *bp = buffer;

	const ubyte *sp = nullptr;
	u8 ss = 0;

	for (; data < data_end; data++) {
		const ubyte c = *data;

		#define XPUTS(str) { \
			sp = (ConstPtr)(str); \
			ss = sizeof(str); \
			goto put_string; \
		}

		if (c < 32) {
			switch (c) {
				case 9:
					if (fmt & FLAG(VStringFmt_KeepHT)) goto put_char;
					break;
				case 10:
					if (fmt & FLAG(VStringFmt_KeepLF)) goto put_char;
					break;
				case 13:
					if (fmt & FLAG(VStringFmt_KeepCR)) goto put_char;
					break;

				default:;
			}

			if (fmt & FLAG(VStringFmt_KeepCTL))
				goto put_char;
			goto put_code;
		}

		if (c <= 129) goto put_char;

		const bool keeputf = fmt & FLAG(VStringFmt_KeepUTF);
		if (fmt & FLAG(VStringFmt_NoUTF)) {
			if (keeputf)
				goto put_char;
			else
				goto put_code;
		}

		// TODO process utf
		XPUTS("&??;")

		#undef XPUTS

		#define XWRITE { \
			auto res = OutStream_write(os, buffer, (usize)(bp - buffer)); \
			if (res) return res; \
			bp = buffer; \
		}

		if (0) put_char: {
			if (fmt & FLAG(VStringFmt_Code))
				goto put_code;

			if (bp + 1 >= buffer_end) XWRITE;
			*(bp++) = c;
		}

		if (0) put_string: {
			if (bp + ss >= buffer_end) XWRITE;
			memcpy(bp, sp, ss);
			bp += ss;
		}

		if (0) put_code: {
			if (bp + 5 >= buffer_end) XWRITE;
			bp[0] = '&';
			if (c < 10) {
				bp[1] = '0' + c;
				bp[2] = ';';
				bp += 3;
			} else if (c < 100) {
				bp[1] = '0' + (c / 10);
				bp[2] = '0' + (c % 10);
				bp[3] = ';';
				bp += 4;
			} else {
				bp[1] = '0' + (c / 100);
				bp[2] = '0' + ((c % 100) / 10);
				bp[3] = '0' + c % 10;
				bp[4] = ';';
				bp += 5;
			}
		}

		#undef XWRITE

	}

	if (bp > buffer)
		return OutStream_write(os, buffer, (usize)(bp - buffer));

	return OutStreamRes_Ok;
}

OutStreamRes VString_print(VString this, PrintFmt fmt, OutStream os) {
	return VString_ZZprint(os,
		String_data(this.value),
		String_size(this.value),
		(VStringFmt)fmt.value
	);
}

OutStreamRes VSmallString_print(VSmallString this, PrintFmt fmt, OutStream os) {
	return VString_ZZprint(os,
		SmallString_data(this.value),
		SmallString_size(this.value),
		(VStringFmt)fmt.value
	);
}
