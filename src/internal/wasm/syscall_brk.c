#include <errno.h>
#include "syscall.h"

long __syscall_brk(long arg1, ...)
{
	return -ENOSYS;
}
