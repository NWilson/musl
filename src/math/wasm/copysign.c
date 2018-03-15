#include <math.h>

__attribute__((const)) double copysign(double x, double y)
{
	return __builtin_copysign(x, y);
}
