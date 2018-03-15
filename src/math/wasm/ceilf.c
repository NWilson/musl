#include <math.h>

__attribute__((const)) float ceilf(float x)
{
	return __builtin_ceilf(x);
}
