#include "pthread_impl.h"

void *__wasm_thread_pointer = 0; // XXX
int __set_thread_area(void *pt)
{
	__wasm_thread_pointer = pt; // XXX __builtin_set_thread_pointer(pt);

	return 1; /* Cannot create threads - yet! */
}
