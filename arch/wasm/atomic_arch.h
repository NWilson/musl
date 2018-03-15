#include <stdint.h>

#define a_ctz_l a_ctz_l
static inline int a_ctz_l(unsigned long x)
{
	return __builtin_ctzl(x);
}

#define a_ctz_64 a_ctz_64
static inline int a_ctz_64(uint64_t x)
{
	return __builtin_ctzll(x);
}

#define a_and_64 a_and_64
static inline void a_and_64(volatile uint64_t *p, uint64_t v)
{
	// TODO use a WebAssembly CAS builtin, when those arrive with the threads feature
	//__atomic_fetch_and(p, v, __ATOMIC_SEQ_CST);
	*p &= v;
}

#define a_or_64 a_or_64
static inline void a_or_64(volatile uint64_t *p, uint64_t v)
{
	// TODO use a WebAssembly CAS builtin, when those arrive with the threads feature
	//__atomic_fetch_or(p, v, __ATOMIC_SEQ_CST);
	*p |= v;
}

#define a_cas a_cas
static inline int a_cas(volatile int *p, int t, int s)
{
	// TODO use a WebAssembly CAS builtin, when those arrive with the threads feature
	//__atomic_compare_exchange_n(p, &t, s, 0, __ATOMIC_SEQ_CST,
	//                            __ATOMIC_SEQ_CST);
	//return t;
	int old = *p;
	if (old == t)
		*p = s;
	return old;
}

#define a_swap a_swap
static inline int a_swap(volatile int *p, int v)
{
	// TODO use a WebAssembly CAS builtin, when those arrive with the threads feature
	//return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST);
	int old = *p;
	*p = v;
	return old;
}

#define a_store a_store
static inline void a_store(volatile int *p, int x)
{
	// TODO use a WebAssembly CAS builtin, when those arrive with the threads feature
	//__atomic_store(p, x, __ATOMIC_RELEASE);
	*p = x;
}

#define a_barrier a_barrier
static inline void a_barrier()
{
	// TODO use a WebAssembly CAS builtin, when those arrive with the threads feature
	//__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

#define a_crash a_crash
static inline void a_crash()
{
	// This generates the Wasm "unreachable" instruction which traps when reached
	__builtin_trap();
}
