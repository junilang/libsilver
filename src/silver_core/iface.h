#ifndef IFACEDB_ALIGN
	#define IFACEDB_ALIGN CPU_HDI_SIZE
#endif

#ifndef IFACEDB_REGISTER_PRIORITY
	#define IFACEDB_REGISTER_PRIORITY 150
#endif

#ifndef IFACEDB_SET_PRIORITY
	#define IFACEDB_SET_PRIORITY 140
#endif

#define IFACEDB(I, Id, open__, rsv__) \
	typedef Id I##_db_Id__; \
	constexpr usize I##_db_open__ = (open__); \
	constexpr usize I##_db_rsv__ = (rsv__); \
	constexpr usize I##_db_slots__ = I##_db_open__ + I##_db_rsv__; \
	struct { \
		usize index; \
		_Alignas(IFACEDB_ALIGN) I db[I##_db_slots__]; \
	} I##_db__ = {}; \
	void I##_db_set__(I##_db_Id__ id, const I iface) { \
		ASSERT(id < I##_db_rsv__, "id not in reserved range"); \
		I##_db__.db[id] = iface; \
	} \
	I##_db_Id__ I##_db_register__(const I iface) { \
		ASSERT(I##_db__.index < I##_db_slots__, "database full"); \
		usize const id = I##_db__.index++; \
		I##_db__.db[id] = iface; \
		return (I##_db_Id__)id; \
	}

#define IFACEDB_REGISTER(I, N) \
	I##_db_Id__ I##_##N##_ID; \
	[[gnu::constructor(150)]] void I##_##N##_db_register_ctor() { \
		I##_##N##_ID = I##_db_register__(I##_##N); \
	}

#define IFACEDB_SET(I, N) \
	[[gnu::constructor(140)]] void I##_##N##_db_set_ctor() { \
		I##_db_set__(I##_##N##_ID, I##_##N); \
	}

#define IFACEDB_GET(I, id) (&I##_db__.db[id])

// ON WINDOWS: pass interfaces as two pointers instead of 16byte struct
// 	on native abi boundaries (function pointers) because the windows abi
// 	is unable to split structs across multiple registers, and passes
// 	a pointer instead

#ifndef IFACESPLIT
	#if !__unix__ // windows
		#define IFACESPLIT true
	#else
		#define IFACESPLIT false
	#endif
#endif

#if IFACESPLIT

	#define IMACRO__(M, B, C, N) M##_##B(C, N)
	#define IMACRO_(M, B, C, N) IMACRO__(M, B, C, N)
	#define IMACRO(M, C, N) IMACRO_(M, C##_PTRTAG, C, N)

	#define IARG_true(C, N) C N
	#define IARG_false(C, N) typeof(((C*)0)->this) N##_this, typeof(((C*)0)->iface) N##_iface
	#define IARG(C, N) IMACRO(IARG, C, N)

	#define IFWD_true(C, N) N
	#define IFWD_false(C, N) N##_this, N##_iface
	#define IFWD(C, N) IMACRO(IFWD, C, N)

	#define IPASS_true(C, N) N
	#define IPASS_false(C, N) N.this, N.iface
	#define IPASS(C, N) IMACRO(IPASS, C, N)

	#define IWRAP_true(C, N) N
	#define IWRAP_false(C, N) ((C){.this=N##_this,.iface=N##_iface})
	#define IWRAP(C, N) IMACRO(IWRAP, C, N)

#else

	#define IARG(C, N) C N
	#define IFWD(C, N) N
	#define IPASS(C, N) N
	#define IWRAP(C, N) N

#endif
