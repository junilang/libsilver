#if WEAVER_USE_PTHREAD
	Ptr WeaverThread_main(Ptr vthis)
#else
	#error "unimplemented"
#endif

{
	WeaverThread *const this = vthis;
	Weaver *rt;
	AsyncTask pending_task = AsyncTask_null;

	u32 state = atom_exchg(
		&this->state, WeaverThreadState_Idle, atom_acq
	);

	#if Weaver_SAFE
		if (state < WeaverThreadState_Boot)
			PANIC("invalid boot state");
	#endif

	


}
