#include <math.h>

__attribute__((const)) double sqrt(double x)
{
	return __builtin_sqrt(x);
}
