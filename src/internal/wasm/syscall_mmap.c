#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#include "atomic.h"
#include "libc.h"
#include "syscall.h"

/* Fixed by the WebAssembly specification, this is the page size
 * used by the underlying brk builtins. */
#define WASM_PAGE_SIZE  65536
#define WASM_PAGE_MASK  (WASM_PAGE_SIZE - 1)

/*
 * How much memory we want to support, as constrained by how many
 * pages we can track using our fixed-size bitmap of mapped
 * pages.  For 32-bit Wasm, we can never support more than 2^31
 * bytes of addressable memory (we want pointer differences not to
 * wrap, see mmap.c).  For 64-bit Wasm, the bitmap approach won't
 * scale to huge amounts of memory... keeping this small seems
 * reasonable for now.
 */
#if defined(__wasm32__)
#define MAX_ADDRESS     0x80000000L  /* 2GiB, the maximum */
#elif defined(__wasm64__)
#define MAX_ADDRESS     0x200000000L /* 8GiB */
#endif

/* Fixed by PAGE_SIZE and MAX_ADDRESS (which are arbitrary): */
#define PAGE_MASK       (PAGE_SIZE - 1)
#define NUM_PAGES       (MAX_ADDRESS / PAGE_SIZE)

static volatile int mmap_lock[2];

#define BITMAP_ELTS     (NUM_PAGES / 64)
#define BITMAP_ELT(x)   (x / 64)
#define BITMAP_BIT(x)   ((uint64_t)1 << (x & 63))
static uint64_t used_pages[BITMAP_ELTS];

static int heap_inited;
static uintptr_t heap_bottom;
static const uintptr_t dummy_heap_bottom;
weak_alias(dummy_heap_bottom, __heap_bottom); // XXX See https://bugs.llvm.org/show_bug.cgi?id=35544


/* For all these, the arguments are page-aligned and the lock is held. */
static void initialize_heap_bottom(void);
static int increase_heap_to(void* end_addr);
static uintptr_t used_pages_test_range(void* addr, size_t length, int set);
static int used_pages_hunt_range(void** addr, size_t length);
static void used_pages_toggle_range(void* addr, size_t length);
static int used_pages_test_mapped(void* addr, size_t length);


/* Check whether we've previously looked for the heap bottom, and
 * initialise it to the current process brk if not. */
static void initialize_heap_bottom(void)
{
	if (heap_inited)
		return;

	/* Find out the heap's position from the special __heap_bottom symbol if it
	 * was given to us, or start from the current memory position otherwise. */
	heap_bottom = __heap_bottom;
	if (!heap_bottom) {
		unsigned long pages = __builtin_wasm_current_memory();
		heap_bottom = (uintptr_t)((pages ? pages : 1) * WASM_PAGE_SIZE);
	}
	heap_bottom = (heap_bottom + PAGE_MASK) & ~PAGE_MASK;

	/* Now allocate those pages so mmap() won't stomp on them. */
	used_pages_toggle_range(0, heap_bottom);
	heap_inited = 1;
}

/* Increase the current process brk if necessary so that heap addresses
 * are mapped up to (but not including) end_addr.  Return true on success. */
static int increase_heap_to(void* end_addr)
{
	unsigned long old_pages, new_pages;
	uintptr_t old_heap_top;

	old_pages = __builtin_wasm_current_memory();
	old_heap_top = old_pages * WASM_PAGE_SIZE;

	if ((uintptr_t)end_addr <= old_heap_top)
		return 1;

	new_pages = (uintptr_t)end_addr / WASM_PAGE_SIZE;
	if ((uintptr_t)end_addr & WASM_PAGE_MASK)
		++new_pages; /* Pedantic rounding up avoiding addition overflow */

	if (__builtin_wasm_grow_memory(new_pages - old_pages) == (unsigned long)-1)
		return 0;

	return 1;
}

/* Return the index of the first page in [addr,addr+length) whose set state
 * differs from "set", or UINTPTR_MAX if all pages in the range match "set". */
static uintptr_t used_pages_test_range(void* addr, size_t length, int set)
{
	/* OK, so imagine that we did some fancy thing to process these 64 bits at
	 * a time... it would hardly matter!  With 32KiB per page, even memset will
	 * take 16 bytes/cycle = thousands of cycles per page, whereas this "slow"
	 * loop here is only a handful of cycles per page. */
	uintptr_t bit_start = (uintptr_t)addr / PAGE_SIZE;
	uintptr_t bit_end = bit_start + length / PAGE_SIZE;

	for (uintptr_t i = bit_start; i < bit_end; ++i) {
		int bit_set = (used_pages[BITMAP_ELT(i)] & BITMAP_BIT(i)) != 0;
		if (bit_set != set)
			return i;
	}

	return UINTPTR_MAX;
}

/* Return true if an unset range of the given length was found, returning it
 * in *addr. */
static int used_pages_hunt_range(void** addr, size_t length)
{
	/* Here it would make sense to be fast!  We may have to traverse up to
	 * 64 thousand pages, just to find a single free page, which doesn't
	 * scale brilliantly.  For now though it should do. */
	for (uintptr_t i = 0; i < BITMAP_ELTS;) {
		uint64_t elt = used_pages[i];
		if (elt == 0xffffffffffffffffull) {
			++i;
			continue;
		}
		for (int j = a_ctz_64(~elt); j < 64;) {
			uintptr_t page_addr = (uint64_t)((i * 64) + j) * PAGE_SIZE;
			if (page_addr > MAX_ADDRESS - length)
				return 0;
			uintptr_t set_page = used_pages_test_range((void*)page_addr,
			                                           length, 0);
			if (set_page == UINTPTR_MAX) {
				*addr = (void*)page_addr;
				return 1;
			}
			/* To avoid n^2 traversal, we have to jump past the "set" page
			 * that we found. */
			uintptr_t next_clear_page = set_page + 1;
			j = next_clear_page - (i * 64);
			i = BITMAP_ELT(next_clear_page);
		}
	}
	return 0;
}

/* Flip the bits in the range [addr,addr+length). */
static void used_pages_toggle_range(void* addr, size_t length)
{
	uintptr_t bit_start = (uintptr_t)addr / PAGE_SIZE;
	uintptr_t bit_end = bit_start + length / PAGE_SIZE;

	/* As for used_pages_test_range, not perf-critical. */
	for (uintptr_t i = bit_start; i < bit_end; ++i) {
		used_pages[BITMAP_ELT(i)] ^= BITMAP_BIT(i);
	}
}

/* Return true if the given range was mapped by mmap(). */
static int used_pages_test_mapped(void* addr, size_t length)
{
	/* Check that the memory was allocated by mmap(), so we don't allow
	 * someone to make us mark it as available! */
	initialize_heap_bottom();
	if ((uintptr_t)addr < heap_bottom)
		return 0;

	if (used_pages_test_range(addr, length, 1) != UINTPTR_MAX)
		return 0;

	return 1;
}


long __syscall_madvise(long arg1, ...)
{
	void* addr;
	size_t length;
	int advice;
	va_list va;

	addr = (void*)arg1;
	va_start(va, arg1);
	length = (size_t)va_arg(va, long);
	advice = (int)va_arg(va, long);
	va_end(va);

	/* Check for one of the advisory-only flags before saying
	 * that we succeeded! */
	if (advice != MADV_NORMAL && advice != MADV_RANDOM &&
	    advice != MADV_SEQUENTIAL && advice != MADV_WILLNEED &&
	    advice != MADV_DONTNEED)
		return -EINVAL;

	if (length > MAX_ADDRESS || length == 0)
		return -EINVAL;
	length = (length + PAGE_MASK) & ~PAGE_MASK;
	if ((uintptr_t)addr & PAGE_MASK)
		return -EINVAL;
	if ((uintptr_t)addr > MAX_ADDRESS - length)
		return -EINVAL;

	LOCK(mmap_lock);

	if (!used_pages_test_mapped(addr, length)) {
		UNLOCK(mmap_lock);
		return -EINVAL;
	}

	UNLOCK(mmap_lock);
	return 0;
}

long __syscall_mmap(long arg1, ...)
{
	void* addr;
	size_t length;
	int prot, flags, fd;
	off_t offset;
	va_list va;

	addr = (void*)arg1;
	va_start(va, arg1);
	length = (size_t)va_arg(va, long);
	prot = (int)va_arg(va, long);
	flags = (int)va_arg(va, long);
	fd = (int)va_arg(va, long);
	offset = (off_t)va_arg(va, long);
	va_end(va);

	/* We only support the MAP_PRIVATE mode. */
	if (!(flags & MAP_PRIVATE) || (flags & MAP_SHARED))
		return -EINVAL;
	flags &= ~(MAP_PRIVATE | MAP_SHARED);
	/*
	 * Flags we support:
	 *    - MAP_ANONYMOUS (and indeed we require it)
	 *    - MAP_FIXED
	 * Everything else is non-POSIX.
	 */
	if (!(flags & MAP_ANONYMOUS) ||
	    (flags & ~(MAP_ANONYMOUS | MAP_FIXED)) ||
	    offset != 0)
		return -EINVAL;
	if (length == 0)
		return -EINVAL;
	if (length > MAX_ADDRESS)
		return -ENOMEM;
	length = (length + PAGE_MASK) & ~PAGE_MASK;
	if (flags & MAP_FIXED) {
		if ((uintptr_t)addr & PAGE_MASK)
			return -EINVAL;
		if ((uintptr_t)addr > MAX_ADDRESS - length)
			return -ENOMEM;
	}
	if (prot & PROT_EXEC)
		return -EPERM;

	LOCK(mmap_lock);

	initialize_heap_bottom();

	if (flags & MAP_FIXED) {
		if (used_pages_test_range(addr, length, 0) != UINTPTR_MAX) {
			UNLOCK(mmap_lock);
			return -ENOMEM;
		}
	} else {
		/* If not using MAP_FIXED, completely ignore the hint and hunt
		 * from the lowest address. */
		if (!used_pages_hunt_range(&addr, length)) {
			UNLOCK(mmap_lock);
			return -ENOMEM;
		}
	}

	if (!increase_heap_to((char*)addr + length)) {
		UNLOCK(mmap_lock);
		return -ENOMEM;
	}
	used_pages_toggle_range(addr, length);

	UNLOCK(mmap_lock);

	memset(addr, 0, length);
	return (long)addr;
}

long __syscall_mremap(long arg1, ...)
{
	void* old_addr, * new_addr;
	size_t old_length, new_length;
	int flags;
	va_list va;

	old_addr = (void*)arg1;
	va_start(va, arg1);
	old_length = (size_t)va_arg(va, long);
	new_length = (size_t)va_arg(va, long);
	flags = (int)va_arg(va, long);
	va_end(va);

	/*
	 * Flags we support:
	 *    - MREMAP_MAYMOVE
	 * MREMAP_FIXED is non-POSIX, not used by Musl, and is a right pain
	 * on top of that to implement (many edge-cases if regions overlap).
	 */
	if (flags & ~(MREMAP_MAYMOVE))
		return -EINVAL;
	if (old_length == 0 || old_length > MAX_ADDRESS)
		return -EINVAL;
	if (new_length > MAX_ADDRESS)
		return -ENOMEM;
	old_length = (old_length + PAGE_MASK) & ~PAGE_MASK;
	new_length = (new_length + PAGE_MASK) & ~PAGE_MASK;
	if ((uintptr_t)old_addr & PAGE_MASK)
		return -EINVAL;
	if ((uintptr_t)old_addr > MAX_ADDRESS - old_length)
		return -EINVAL;

	if (new_length < old_length) {
		long rv = __syscall_munmap((long)((char*)old_addr + new_length),
		                           old_length - new_length);
		return rv == 0 ? (long)old_addr : rv;
	}

	LOCK(mmap_lock);

	if (!used_pages_test_mapped(old_addr, old_length)) {
		UNLOCK(mmap_lock);
		return -EINVAL;
	}

	/* Trivial case; can't handle via munmap() since that checks
	 * length != 0, and can't handle before lock because we must
	 * check the range is actually mapped before returning success. */
	if (new_length == old_length) {
		UNLOCK(mmap_lock);
		return (long)old_addr;
	}

	/* Check if we can do a cheap expansion.  If increasing the heap brk
	 * fails, continue below to try moving, since there may be a large
	 * enough free area lower down to move into. */
	if (used_pages_test_range((char*)old_addr + old_length,
	                          new_length - old_length, 0) == UINTPTR_MAX &&
	    increase_heap_to((char*)old_addr + new_length)) {
		used_pages_toggle_range((char*)old_addr + old_length,
		                        new_length - old_length);

		UNLOCK(mmap_lock);

		memset((char*)old_addr + old_length, 0,
		       new_length - old_length);
		return (long)old_addr;
	}

	if (!(flags & MREMAP_MAYMOVE)) {
		UNLOCK(mmap_lock);
		return -ENOMEM;
	}

	if (!used_pages_hunt_range(&new_addr, new_length) ||
	    !increase_heap_to((char*)new_addr + new_length)) {
		UNLOCK(mmap_lock);
		return -ENOMEM;
	}

	used_pages_toggle_range(new_addr, new_length);

	memcpy(new_addr, old_addr, old_length);
	memset((char*)new_addr + old_length, 0, new_length - old_length);
	used_pages_toggle_range(old_addr, old_length);

	UNLOCK(mmap_lock);
	return (long)new_addr;
}

long __syscall_munmap(long arg1, ...)
{
	void* addr;
	size_t length;
	va_list va;

	addr = (void*)arg1;
	va_start(va, arg1);
	length = (size_t)va_arg(va, long);
	va_end(va);

	if (length > MAX_ADDRESS || length == 0)
		return -EINVAL;
	length = (length + PAGE_MASK) & ~PAGE_MASK;
	if ((uintptr_t)addr & PAGE_MASK)
		return -EINVAL;
	if ((uintptr_t)addr > MAX_ADDRESS - length)
		return -EINVAL;

	LOCK(mmap_lock);

	if (!used_pages_test_mapped(addr, length)) {
		UNLOCK(mmap_lock);
		return -EINVAL;
	}

	used_pages_toggle_range(addr, length);

	UNLOCK(mmap_lock);
	return 0;
}
