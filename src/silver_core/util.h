#define UNIQUEPTR(name) struct {char name##_uniqueptr__;} *name
#define UNIQUENUM(T, name) enum : T {name##_uniquenum__} name

#define STRUCTDECL(name) struct name name
#define UNIONDECL(name) union name name

#define LITERAL(T,...) ((T){__VA_ARGS__})

#define STR(str) (ConstPtr)(str), __builtin_strlen(str)

#define COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
	const ubyte *data;
	usize size;
} String;

#define String_INIT(str) {.data=(ConstPtr)(str), .size=(__builtin_strlen(str))}

#if __clang__
	#define LIKELY [[clang::likely]]
	#define UNLIKELY [[clang::unlikely]]
#else
	#define LIKELY
	#define UNLIKELY
#endif
