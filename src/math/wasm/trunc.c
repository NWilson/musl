#include <math.h>

__attribute__((const)) double trunc(double x)
{
	return __builtin_trunc(x);
}
