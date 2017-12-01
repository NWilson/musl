#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
// The WebAssembly fixed page size is 64KiB
#define PAGE_SIZE 65536
#define LONG_BIT (__SIZEOF_LONG__*8)
#endif

#define LONG_MAX  __LONG_MAX__
#define LLONG_MAX __LONG_LONG_MAX__
