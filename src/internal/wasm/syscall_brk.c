#include <limits.h>
#include "syscall.h"

long __syscall_brk(long arg1, ...)
{
	unsigned long newbrk = (unsigned long)arg1;
	unsigned long pages = __builtin_wasm_current_memory();
	if (newbrk % PAGE_SIZE)
		goto end;
	unsigned long new_pages = newbrk / PAGE_SIZE;
	if (new_pages <= pages || new_pages >= (0xffffffffu / PAGE_SIZE))
		goto end;
	if (__builtin_wasm_grow_memory(new_pages - pages) != (unsigned long)-1)
		pages = new_pages;
end:
	return pages * PAGE_SIZE;
}
