// Verbose string - used for analyzing utf8 strings

typedef u8 VStringFmt; enum {
	FLAG_DEF(VStringFmt_Code),
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

	ubyte c;

	for (; data < data_end; data++) {
		c = *data;

		goto put_char;

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

		/*
		if (0) put_string: {
			if (bp + s.size >= buffer_end) XWRITE;

			memcpy(bp, s.data, s.size);
			bp += s.size;
		}
		*/

		if (0) put_code: {
			if (c < 10) {
				if (bp + 3 >= buffer_end) XWRITE;
				bp[0] = '&';
				bp[1] = '0' + c;
				bp[2] = ';';
				bp += 3;
			} else if (c < 100) {
				if (bp + 4 >= buffer_end) XWRITE;
				bp[0] = '&';
				bp[1] = '0' + (c / 10);
				bp[2] = '0' + (c % 10);
				bp[3] = ';';
				bp += 4;
			} else {
				if (bp + 5 >= buffer_end) XWRITE;
				bp[0] = '&';
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
