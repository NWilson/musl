#include <errno.h>
#include "syscall.h"

// Wasm doesn't have mmap!  There's just a single linear memory block.

// File mapping:
//map = __mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
// malloc:
//void *area = __mmap(0, n, PROT_READ|PROT_WRITE,
//                    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
// expand:
//char *base = __mmap(0, len, PROT_READ|PROT_WRITE,
//                    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
// generic:
//ret = __syscall(SYS_mmap, start, len, prot, flags, fd, off);
// TLS:
//map = __mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
//map = __mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
//__mprotect(map+guard, size-guard, PROT_READ|PROT_WRITE)
// sem_open:
//mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)

long __syscall_madvise(long arg1, ...)
{
	// mark pages as being unneeded... (save backing memory)
	//__madvise((void *)a, b-a, MADV_DONTNEED);
	return 0;
}
long __syscall_mmap(long arg1, ...)
{
	return -ENOSYS;
}
long __syscall_mremap(long arg1, ...)
{
	return -ENOSYS;
}
long __syscall_munmap(long arg1, ...)
{
	return 0;
}
