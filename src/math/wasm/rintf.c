#include <math.h>

__attribute__((const)) float rintf(float x)
{
	return __builtin_rintf(x);
}
