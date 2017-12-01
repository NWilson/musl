#include <futex.h>
#include <stdarg.h>
#include <errno.h>
#include "syscall.h"

// Wasm doesn't *yet* have futex(), but it's being planned as part of the
// threaded-Wasm support in Spring 2018.
//
// For now, Wasm is single-threaded and we simply assert that the lock is not
// held, and abort if a wait would be required (assume it's a corrupted lock).

long __syscall_futex(long arg1, ...)
{
	va_list va;
	va_start(va, arg1);

	volatile int* addr = (volatile int*)arg1;
	long op = va_arg(va, long);

	op &= ~FUTEX_PRIVATE;

	if (op == FUTEX_WAIT) {
		int val = (int)va_arg(va, long);
		// arg4 would be the timeout as a timespec*
		va_end(va);

		if (*addr == val) {
			// trap, Wasm can't block
			// TODO use a WebAssembly futex builtin, when those arrive!
			__builtin_unreachable();
		}
		return 0;
	}
	if (op == FUTEX_WAKE) {
		// arg3 would be the number of waiters to wake as an int
		va_end(va);

		// Wasm can't block/wait
		// TODO use a WebAssembly futex builtin, when those arrive!
		return 0;
	}

	va_end(va);
	return -ENOSYS;
}
