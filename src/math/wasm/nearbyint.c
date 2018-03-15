#include <math.h>

__attribute__((const)) double nearbyint(double x)
{
	return __builtin_nearbyint(x);
}
