#ifndef _WASM_INIT_DATA_H
#define _WASM_INIT_DATA_H

#include <stdlib.h>

struct wasm_init_data_t {
	long argc;
	const char* argv0;
	const char* argv1;
	const char* envp0;
	size_t aux[2*5];
};
extern struct wasm_init_data_t wasm_init_data;

#endif
