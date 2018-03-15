#include <stdarg.h>
#include "syscall.h"

/* This is hardly required at all... in all but one place in the
 * Musl codebase, the __syscall macro is able to redirect to
 * __syscallN directly, but in __syscall_cp there's an exception
 * where the varargs version is called explicitly.  We therefore
 * need this extra stub.
 *
 * TODO See if we can get this stub accepted in src/internal/syscall.c
 * so it's not necessary to do this in the Wasm arch code.  On the
 * other hand, most arches don't allow __syscall to be a "normal"
 * function, so it's somewhat appropriate for Wasm to add its own
 * stub here.
 */
long (__syscall)(long n, ...)
{
	va_list va;
	va_start(va, n);
	long a1 = va_arg(va, long);
	long a2 = va_arg(va, long);
	long a3 = va_arg(va, long);
	long a4 = va_arg(va, long);
	long a5 = va_arg(va, long);
	long a6 = va_arg(va, long);
	return __syscall6(n, a1, a2, a3, a4, a5, a6);
}
