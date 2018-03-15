#include <math.h>

__attribute__((const)) float truncf(float x)
{
	return __builtin_truncf(x);
}
