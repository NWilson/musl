#include "syscall.h"

// Wasm doesn't *yet* have threading support, but it's being planned as part
// of the threaded-Wasm support in Spring 2018.

long __syscall_set_tid_address(long arg1, long arg2, long arg3,
                               long arg4, long arg5, long arg6)
{
	// SYS_set_tid_address stashes away a pointer that won't be touched until
	// the thread exits (never in Wasm currently), and returns the thread ID
	// (dummy zero).

	// int* ctid = (int*)arg1;
	return 0;
}
