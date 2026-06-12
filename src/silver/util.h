#define UNIQUEPTR(name) struct {char name##_uniqueptr__[];} *name
#define UNIQUENUM(T, name) enum : T {name##_uniquenum__} name

#define STRUCTDECL(name) struct name name
#define UNIONDECL(name) union name name

#define LITERAL(T,...) ((T){__VA_ARGS__})
