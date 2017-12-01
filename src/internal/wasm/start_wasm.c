#include <wasm/wasm_init_data.h>

extern void __libc_start_init(void);
extern void __init_libc(char **envp, char *pn);

__attribute__((visibility("default")))
void _start_wasm()
{
	__init_libc((char**)&wasm_init_data.envp0, (char*)wasm_init_data.argv0);
	__libc_start_init();
}
