#include <math.h>

__attribute__((const)) double floor(double x)
{
	return __builtin_floor(x);
}
