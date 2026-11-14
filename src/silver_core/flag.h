#define FLAG_DEF(flag) flag##_FLAG

#define FLAG(flag) (1ull << flag##_FLAG)
#define FLAG_CLEAR(flag) (~FLAG(flag))
#define FLAG_NOT(flag) FLAG_CLEAR(flag)

#define FLAGS_HEADER_OPT
#define FLAGS_HEADER_ FLAGS_ERROR_NO_ARGS

#define FLAGS_(pfx, ...) (FLAGS_HEADER_##__VA_OPT__(OPT FLAGS_0(pfx, __VA_ARGS__)))

#define FLAGS(pfx, ...) FLAGS_(pfx##_, __VA_ARGS__)
#define FLAGS_CAST(pfx, ...) ((pfx)FLAGS(pfx, __VA_ARGS__))
#define FLAGS_CLEAR(pfx, ...) (~(FLAGS(pfx, __VA_ARGS__)))
#define FLAGS_CLEAR_CAST(pfx, ...) ((pfx)FLAGS_CLEAR(pfx, __VA_ARGS__))
#define FLAGS_NOT FLAGS_CLEAR
#define FLAGS_NOT_CAST FLAGS_CLEAR_CAST

#define FLAGS_DEF_(pfx, ...) FLAGS_HEADER_##__VA_OPT__(OPT FLAGS_DEF_0(pfx, __VA_ARGS__))
#define FLAGS_DEF(pfx, ...) FLAGS_DEF_(pfx##_, __VA_ARGS__)

#define FLAGS_0(pfx, flag, ...) FLAG(pfx##flag)__VA_OPT__(|FLAGS_1(pfx, __VA_ARGS__))
#define FLAGS_DEF_0(pfx, flag, ...) FLAG_DEF(pfx##flag)__VA_OPT__(,FLAGS_DEF_1(pfx, __VA_ARGS__))

#define FLAGS_ERROR_TOO_MANY_FLAGS _Pragma("GCC error \"FLAGS: too many flags\"")
#define FLAGS_ERROR_NO_ARGS _Pragma("GCC error \"FLAGS: no args\"")

#include "flag_gen.h"
