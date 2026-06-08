typedef unsigned long long AnyUint;
typedef long long AnyInt;

enum {
	AnyIntFmt_BIT_Negative,
};

typedef u32 AnyIntFmt;

#define AnyInt_BUFSIZE 64

String AnyUint_tostr_bigbuf(AnyUint val, AnyIntFmt fmt, ubyte buf[AnyInt_BUFSIZE]) {
	auto be = buf + (AnyInt_BUFSIZE / 2);
	auto bp = buf + (AnyInt_BUFSIZE / 2);

	if (val == 0) {
		*(--bp) = '0';
	} else while (val > 0) {
		*(--bp) = '0' + (ubyte)(val % 10);
		val /= 10;
	}

	if (fmt & FLAG(AnyIntFmt, Negative)) {
		*(--bp) = '-';
	}

	return (String){.data=bp, .size=(usize)(be - bp)};
}

String AnyInt_tostr_bigbuf(AnyInt val, AnyIntFmt fmt, ubyte buf[AnyInt_BUFSIZE]) {
	if (val < 0) {
		fmt |= FLAG(AnyIntFmt, Negative);
		val = -val;
	}
	return AnyUint_tostr_bigbuf((AnyUint)val, fmt, buf);
}

void AnyInt_print(AnyInt val, PrintFmt fmt, OutStream os) {
	ubyte buf[AnyInt_BUFSIZE];
	String_print(
		AnyInt_tostr_bigbuf(val, fmt.value, buf),
		PrintFmt_NULL, os
	);
}

void AnyUint_print(AnyUint val, PrintFmt fmt, OutStream os) {
	ubyte buf[AnyInt_BUFSIZE];
	String_print(
		AnyUint_tostr_bigbuf(val, fmt.value, buf),
		PrintFmt_NULL, os
	);
}

void AnyUintPtr_print(AnyUint *val, PrintFmt fmt, OutStream os) {
	AnyUint_print(*val, fmt, os);
}

void AnyIntPtr_print(AnyInt *val, PrintFmt fmt, OutStream os) {
	AnyInt_print(*val, fmt, os);
}

IPrintable_GENERATE_KNOWN(AnyIntPtr, AnyInt*)
IPrintable_GENERATE_KNOWN(AnyUintPtr, AnyUint*)
