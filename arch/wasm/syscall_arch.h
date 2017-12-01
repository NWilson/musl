// For both Wasm32 and Wasm64, we assume that the host environment can't
// provide 64-bit types, and split 64-bit values into two arguments.  A Wasm
// interpreter *could* provide i64 support, but in practice web browsers aren't
// doing that right now, and any i64 we pass out to a syscall will get chopped
// to 58-bit precision.
#define __SYSCALL_LL_E(x) \
((union { long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) __SYSCALL_LL_E((x))

static inline long __syscall0(long n)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(0);
}

static inline long __syscall1(long n, long a1)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(a1);
}

static inline long __syscall2(long n, long a1, long a2)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(a1,a2);
}

static inline long __syscall3(long n, long a1, long a2, long a3)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(a1,a2,a3);
}

static inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(a1,a2,a3,a4);
}

static inline long __syscall5(long n, long a1, long a2, long a3, long a4,
                              long a5)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(a1,a2,a3,a4,a5);
}

static inline long __syscall6(long n, long a1, long a2, long a3, long a4,
                              long a5, long a6)
{
	long (*fn)(long,...) = (long (*)(long,...))n;
	return fn(a1,a2,a3,a4,a5,a6);
}

#undef SYSCALL_STATIC
#define SYSCALL_STATIC 1

#define SYSCALL_FADVISE_6_ARG
