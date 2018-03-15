#include <wasm/wasm_init_data.h>

// TODO: We're sneakily declaring this function here since it isn't in a
// header. Apparently that means other parts of Musl shouldn't therefore
// be calling it, as that would constitute an "internal API" contract.
// Ideally we'd obtain "permission" to call it here.
extern void __init_libc(char **envp, char *pn);

// Perform libc initialisation at the highest priority.  Wasm doesn't
// get its entrypoint from libc, instead the linker synthesises it
// from all the global C and C++ constructors.  The job of calling "main"
// (if there is such a function) is up to the application.
__attribute__((constructor(1)))
void __libc_ctor()
{
	const struct wasm_init_data_t *init_data = __wasm_get_init_data();
	__init_libc((char**)&init_data->envp0, (char*)init_data->argv0);
}
