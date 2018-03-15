#ifndef _WASM_INIT_DATA_H
#define _WASM_INIT_DATA_H

#include <stdlib.h>

struct wasm_init_data_t {
	long argc;
	const char* argv0;
	const char* argv1;
	const char* envp0;
	/* Following data matches: size_t aux[]; */
};
const struct wasm_init_data_t *__wasm_get_init_data(void);

#endif
