#include <wasm/wasm_init_data.h>
#include <bits/hwcap.h>
#include <bits/limits.h>
#include <elf.h>

static const char* const progname = "/a.out";

extern const uintptr_t __tls_base __attribute__((weak));
extern const uintptr_t __tls_data_size __attribute__((weak));
extern const uintptr_t __tls_bss_size __attribute__((weak));
extern const uintptr_t __tls_align __attribute__((weak));

#if ULONG_MAX == 0xffffffff
static Elf32_Phdr phdrs[1];
#else
static Elf64_Phdr phdrs[1];
#endif

static struct {
	struct wasm_init_data_t data;
	size_t aux[8*2];
} init_data = {
	{
		.argc  = 1,
		.argv0 = progname,
		.argv1 = 0,
		.envp0 = 0,
	},
};

const struct wasm_init_data_t *__wasm_get_init_data(void)
{
	int auxi = 0, pi = 0;
	// It would be nice if Wasm provided AT_RANDOM, oh well.  The rest
	// can just be dummies.
	init_data.aux[auxi++] = AT_HWCAP;
	init_data.aux[auxi++] = HWCAP_MVP;
	init_data.aux[auxi++] = AT_PAGESZ;
	init_data.aux[auxi++] = PAGE_SIZE;
	init_data.aux[auxi++] = AT_EXECFN;
	init_data.aux[auxi++] = (uintptr_t)progname;
#if defined __wasm32__
	init_data.aux[auxi++] = AT_PLATFORM;
	init_data.aux[auxi++] = (uintptr_t)"wasm32";
#elif defined __wasm64__
	init_data.aux[auxi++] = AT_PLATFORM;
	init_data.aux[auxi++] = (uintptr_t)"wasm64";
#endif
	if (&__tls_base && &__tls_align) {
		phdrs[pi].p_type   = PT_TLS,
		phdrs[pi].p_flags  = PF_R,
		phdrs[pi].p_offset = 0, // File offset, ignored in Wasm
		phdrs[pi].p_vaddr  = (uintptr_t)&__tls_base,
		phdrs[pi].p_paddr  = (uintptr_t)0, // 0 for TLS, unused
		phdrs[pi].p_filesz = (uintptr_t)&__tls_data_size,
		phdrs[pi].p_memsz  = (uintptr_t)&__tls_data_size + (uintptr_t)&__tls_bss_size,
		phdrs[pi].p_align  = (uintptr_t)&__tls_align,
		init_data.aux[auxi++] = AT_PHDR;
		init_data.aux[auxi++] = (uintptr_t)&phdrs;
		init_data.aux[auxi++] = AT_PHENT;
		init_data.aux[auxi++] = sizeof(*phdrs);
		init_data.aux[auxi++] = AT_PHNUM;
		init_data.aux[auxi++] = (sizeof(phdrs)/sizeof(*phdrs));
	}
	init_data.aux[auxi++] = AT_NULL;
	init_data.aux[auxi++] = 0;
	return &init_data.data;
}

