#include <wasm/wasm_init_data.h>
#include <bits/hwcap.h>
#include <bits/limits.h>
#include <elf.h>

static const char* const progname = "/a.out";

struct wasm_init_data_t wasm_init_data = {
	.argc = 1,
	.argv0 = progname,
	.argv1 = 0,
	.envp0 = 0,
	.aux = {
		// It would be nice if Wasm provided AT_RANDOM, oh well.  The rest
		// can just be dummies.
		AT_HWCAP,    HWCAP_MVP,
		AT_PAGESZ,   PAGE_SIZE,
		AT_EXECFN,   (uintptr_t)progname,
#if defined __wasm32__
		AT_PLATFORM, (uintptr_t)"wasm32",
#elif defined __wasm64__
		AT_PLATFORM, (uintptr_t)"wasm64",
#endif
		AT_NULL,     0
	}
};
