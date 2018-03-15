#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
/*
 * How big should our pages be?  There is *no requirement at all* that our
 * emulated kernel page size matches the underlying page size used by the
 * Wasm memory builtins!  Wasm pages are 64KiB, which is pretty huge.
 * malloc() switches to using mmap around the 128KiB mark, which means a
 * 129KiB allocation would be wasting 33% of the RAM if we did 64KiB mmap
 * allocations.  At 32KiB page size, we have <20% RAM
 * waste which seems a bit more reasonable.  All a bit arbitrary.
 */
#define PAGE_SIZE 32768
#define LONG_BIT (__SIZEOF_LONG__*8)
#endif

#define LONG_MAX  __LONG_MAX__
#define LLONG_MAX __LONG_LONG_MAX__
