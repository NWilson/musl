#ifndef SHARED

#include <wasm/wasm_init_data.h>

extern void _start_c(long* p);
void _start(void)
{
	_start_c(&wasm_init_data.argc);
}

#endif