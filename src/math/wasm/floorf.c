#include <math.h>

__attribute__((const)) float floorf(float x)
{
	return __builtin_floorf(x);
}
