#ifndef Asynx_PFX
	#define Asynx_PFX async
#endif

#ifndef Asynx_LOCALS
	#define Asynx_LOCALS Asynx_LOCAL_THIS
#endif

#define async_sizeof Asynx_SIZEOF
#define async_alingof Asynx_ALIGNOF

#define async_create Asynx_CREATE
#define async_create_resolve Asynx_CREATE_RESOLVE
#define async_set_resolve Asynx_SET_RESOLVE

#define async_state Asynx_STATE
#define async_labels Asynx_LABELS

#define async_begin Asynx_BEGIN
#define async_end Asynx_END

#define async_label Asynx_LABEL
#define async_self Asynx_SELF

#define async_goto Asynx_GOTO
#define async_yield Asynx_YIELD
#define async_return Asynx_RETURN

#define async_suspend_to Asynx_SUSPEND_TO
#define async_suspend Asynx_SUSPEND

#define async_resume_to Asynx_RESUME_TO
#define async_resume Asynx_RESUME

#define async_call_to Asynx_CALL_RESUME_TO
#define async_call Asynx_CALL_RESUME

#define async_call_suspend_to Asynx_CALL_SUSPEND_TO
#define async_call_suspend Asynx_CALL_SUSPEND

#define async_call_join_to Asynx_CALL_JOIN_RESUME_TO
#define async_call_join Asynx_CALL_JOIN_RESUME

#define async_call_join_suspend_to Asynx_CALL_JOIN_SUSPEND_TO
#define async_call_join_suspend Asynx_CALL_JOIN_SUSPEND

#define async_spawn_with Asynx_SPAWN_WITH
#define async_spawn Asynx_SPAWN

#define async_wait_to Asynx_WAIT_IMMEDIATE_TO
#define async_wait Asynx_WAIT_IMMEDIATE

#define async_wait_resume_to Asynx_WAIT_RESUME_TO
#define async_wait_resume Asynx_WAIT_RESUME

#define async_wait_suspend_to Asynx_WAIT_SUSPEND_TO
#define async_wait_suspend Asynx_WAIT_SUSPEND

#define async_fork_to Asynx_FORK_IMMEDIATE_TO
#define async_fork Asynx_FORK_IMMEDIATE

#define async_fork_resume_to Asynx_FORK_RESUME_TO
#define async_fork_resume Asynx_FORK_RESUME

#define async_fork_suspend_to Asynx_FORK_SUSPEND_TO
#define async_fork_suspend Asynx_FORK_SUSPEND

#define async_join_to Asynx_JOIN_IMMEDIATE_TO
#define async_join Asynx_JOIN_IMMEDIATE

#define async_join_resume_to Asynx_JOIN_RESUME_TO
#define async_join_resume Asynx_JOIN_RESUME

#define async_join_suspend_to Asynx_JOIN_SUSPEND_TO
#define async_join_suspend Asynx_JOIN_SUSPEND
