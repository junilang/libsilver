#define UNIQUEPTR(name) struct {char name##_uniqueptr__;} *name
#define UNIQUENUM(T, name) enum : T {name##_uniquenum__} name

#define STRUCTDECL(name) struct name name
#define UNIONDECL(name) union name name

#define LITERAL(T,...) ((T){__VA_ARGS__})

#define STR(str) (ConstPtr)(str), (sizeof(str) - 1)

typedef struct {
	const ubyte *data;
	usize size;
} String;

#define String_INIT(str) {.data=(ConstPtr)(str), .size=(sizeof(str) - 1)}
