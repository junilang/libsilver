#define ENUM(PFX, TYPE, BASE, N, ...) \
	enum : TYPE { PFX##N = BASE, __VA_OPT__(ENUM_1(PFX, __VA_ARGS__)) }

#define ENUM_1(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_2(PFX, __VA_ARGS__))
#define ENUM_2(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_3(PFX, __VA_ARGS__))
#define ENUM_3(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_4(PFX, __VA_ARGS__))
#define ENUM_4(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_5(PFX, __VA_ARGS__))
#define ENUM_5(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_6(PFX, __VA_ARGS__))
#define ENUM_6(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_7(PFX, __VA_ARGS__))
#define ENUM_7(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_8(PFX, __VA_ARGS__))
#define ENUM_8(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_9(PFX, __VA_ARGS__))
#define ENUM_9(PFX, N, ...) PFX##N, __VA_OPT__(ENUM_10(PFX, __VA_ARGS__))
#define ENUM_10(PFX, N, ...) GCC_ERROR_MAX_DEPTH_REACHED
