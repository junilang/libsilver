// Verbose string - used for analyzing utf8 strings

enum {
	FLAG_DEF(VStringFmt_NoUTF),
	FLAG_DEF(VStringFmt_KeepUTF),
	FLAG_DEF(VStringFmt_KeepHT),
	FLAG_DEF(VStringFmt_KeepCR),
	FLAG_DEF(VStringFmt_KeepLF),
};

typedef struct {
	String value;
} VString;

typedef struct {
	SmallString value;
} VSmallString;
