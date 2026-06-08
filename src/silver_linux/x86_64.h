#define LINUX_SYSCALL_REG_0 "rax"
#define LINUX_SYSCALL_REG_1 "rdi"
#define LINUX_SYSCALL_REG_2 "rsi"
#define LINUX_SYSCALL_REG_3 "rdx"
#define LINUX_SYSCALL_REG_4 "r10"
#define LINUX_SYSCALL_REG_5 "r8"
#define LINUX_SYSCALL_REG_6 "r9"

#define LINUX_SYSCALL_REG_RESULT 0

#define LINUX_SYSCALL_CLOBBER_LIST "rcx", "r11", "memory"

// assuming LP64

#define WORD_WIDTH 64

typedef unsigned long uword;
static_assert(sizeof(uword) == 8);

constexpr uword uword_max = (uword)(~0ull);
constexpr __UINT8_TYPE__ uword_width = WORD_WIDTH;


typedef signed long iword;
static_assert(sizeof(iword) == 8);

constexpr iword iword_max = (iword)(uword_max >> 1);
constexpr iword iword_min = -iword_max - 1;
constexpr __UINT8_TYPE__ iword_width = WORD_WIDTH;
