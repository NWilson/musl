#include <math.h>

__attribute__((const)) float sqrtf(float x)
{
	return __builtin_sqrtf(x);
}
