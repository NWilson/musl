#include <math.h>

__attribute__((const)) double ceil(double x)
{
	return __builtin_ceil(x);
}
