#include <math.h>

__attribute__((const)) double rint(double x)
{
	return __builtin_rint(x);
}
