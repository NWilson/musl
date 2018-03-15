#include <math.h>

__attribute__((const)) double fabs(double x)
{
	return __builtin_fabs(x);
}
