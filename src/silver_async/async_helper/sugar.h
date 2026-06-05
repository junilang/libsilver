#ifndef async_PFX
	#define async_PFX async
#endif

#ifndef async_LOCALS
	#define async_LOCALS async_LOCAL_THIS
#endif

#define async_allocsize async_ALLOCSIZE

#define async_create async_CREATE
#define async_create_resolve async_CREATE_RESOLVE
#define async_set_resolve async_SET_RESOLVE

#define async_state async_STATE
#define async_labels async_LABELS

#define async_begin async_BEGIN
#define async_end async_END

#define async_label async_LABEL
#define async_self async_SELF

#define async_goto async_GOTO
#define async_yield async_YIELD
#define async_return async_RETURN

#define async_suspend_to async_SUSPEND_TO
#define async_suspend async_SUSPEND

#define async_resume_to async_RESUME_TO
#define async_resume async_resume

#define async_call_to async_CALL_TO
#define async_call async_CALL

#define async_call_suspend_to async_CALL_SUSPEND_TO
#define async_call_suspend async_CALL_SUSPEND

#define async_call_join_to async_CALL_JOIN_RESUME_TO
#define async_call_join async_CALL_JOIN_RESUME

#define async_call_join_suspend_to async_CALL_JOIN_SUSPEND_TO
#define async_call_join_suspend async_CALL_JOIN_SUSPEND

#define async_spawn_with async_SPAWN_WITH
#define async_spawn async_SPAWN

#define async_wait_to async_WAIT_IMMEDIATE_TO
#define async_wait async_WAIT_IMMEDIATE

#define async_wait_resume_to async_WAIT_RESUME_TO
#define async_wait_resume async_WAIT_RESUME

#define async_wait_suspend_to async_WAIT_SUSPEND_TO
#define async_wait_suspend async_WAIT_SUSPEND

#define async_fork_to async_FORK_IMMEDIATE_TO
#define async_fork async_FORK_IMMEDIATE

#define async_fork_resume_to async_FORK_RESUME_TO
#define async_fork_resume async_FORK_RESUME

#define async_fork_suspend_to async_FORK_SUSPEND_TO
#define async_fork_suspend async_FORK_SUSPEND

#define async_join_to async_JOIN_IMMEDIATE_TO
#define async_join async_JOIN_IMMEDIATE

#define async_join_resume_to async_JOIN_RESUME_TO
#define async_join_resume async_JOIN_RESUME

#define async_join_suspend_to async_JOIN_SUSPEND_TO
#define async_join_suspend async_JOIN_SUSPEND
