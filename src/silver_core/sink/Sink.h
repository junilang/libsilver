#ifndef Sink_PTRTAG
	#define Sink_PTRTAG PTRTAG
#endif

#ifndef Sink_SAFE
	#define Sink_SAFE LIBSILVER_SAFE
#endif

#include "SinkReq.h"
#include "SinkRes.h"

typedef SinkRes (*ISink)(Ptr this, SinkReq req, Ptr aux, ConstPtr src);

#if Sink_PTRTAG
	typedef struct {
		union {
			Ptr value;
			usize raw_value;
		};
	} Sink;

	enum {
		ISink_VoidSink_ID,
		ISink_LockSink_ID,
		ISink_BufferSink_ID,
		ISink_BufferedSink_ID,
		ISink_AsyncSink_ID,
		ISink_OsSink_ID,
		ISink_reserved
	};

	#ifndef ISink_SLOTS
		#define ISink_SLOTS 8
	#endif

	constexpr auto ISink_slots = ISink_SLOTS;

	IFACEDB(ISink, utag, ISink_slots, ISink_reserved)

	Ptr Sink_this(Sink this) {
		return ptrstrip(this.value);
	}

	ISink Sink_iface(Sink this) {
		return *IFACEDB_GET(ISink, ptrread(this.value));
	}

	#define CONSTEXPR_Sink_upcast(N, V) \
		{.raw_value=CONSTEXPR_ptrtag((V), ISink_##N##_ID)}
#else
	typedef struct {
		Ptr this;
		ISink iface;
	} Sink;

	Ptr Sink_this(Sink this) { return this.this; }
	ISink Sink_iface(Sink this) { return this.iface; }

	#define CONSTEXPR_Sink_upcast(N, V) \
		{.this=(Ptr)(usize)(V), .iface=&ISink_##N}

#endif

[[nodiscard]]
SinkRes Sink_invoke(Sink this, SinkReq req, Ptr aux, ConstPtr src) {
	return Sink_iface(this)(Sink_this(this), req, aux, src);
}

SinkAttr Sink_attr(Sink this) {
	SinkAttr result;

	[[maybe_unused]]
	auto const res = Sink_invoke(this, FIELD(SinkIntent, Attr), &result, nullptr);
	#if Sink_SAFE
		SinkRes_ASSERT(res);
	#endif

	return result;
}

[[nodiscard]]
SinkRes Sink_write(Sink this, ConstPtr buf, usize buf_size) {
	#if Sink_SAFE
		ASSERT(buf_size <= SinkSize_max);
	#endif
	return Sink_invoke(this,
		FIELD(SinkIntent, Write) | FIELD_SET(SinkSize, buf_size)
	, nullptr, buf);
}

[[nodiscard]]
SinkRes Sink_writeflush(Sink this, ConstPtr buf, usize buf_size) {
	#if Sink_SAFE
		ASSERT(buf_size <= SinkSize_max);
	#endif
	return Sink_invoke(this,
		FIELD(SinkIntent, Write) | FLAG(SinkFlag_Flush) | FIELD_SET(SinkSize, buf_size)
	, nullptr, buf);
}

[[nodiscard]]
SinkRes Sink_writepush(Sink this, ConstPtr buf, usize buf_size) {
	#if Sink_SAFE
		ASSERT(buf_size <= SinkSize_max);
	#endif
	return Sink_invoke(this,
		FIELD(SinkIntent, Write) | FLAG(SinkFlag_Push) | FIELD_SET(SinkSize, buf_size)
	, nullptr, buf);
}

[[nodiscard]]
SinkRes Sink_flush(Sink this) {
	return Sink_invoke(this,
		FIELD(SinkIntent, Flush)
	, nullptr, nullptr);
}

[[nodiscard]]
SinkRes Sink_push(Sink this) {
	return Sink_invoke(this,
		FIELD(SinkIntent, Push)
	, nullptr, nullptr);
}

[[nodiscard]]
SinkRes Sink_lock(Sink this) {
	return Sink_invoke(this,
		FIELD(SinkIntent, Lock)
	, nullptr, nullptr);
}

[[nodiscard]]
SinkRes Sink_trylock(Sink this) {
	return Sink_invoke(this,
		FIELD(SinkIntent, Lock) | FLAG(SinkFlag_NoWait)
	, nullptr, nullptr);
}

[[nodiscard]]
SinkRes Sink_unlock(Sink this) {
	return Sink_invoke(this,
		FIELD(SinkIntent, Unlock)
	, nullptr, nullptr);
}

#include "Sink_meta.h"

extern Sink os_panic_sink();
extern Sink os_stderr_sink();
extern Sink os_stdout_sink();
