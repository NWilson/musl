#include <math.h>

__attribute__((const)) float copysignf(float x, float y)
{
	return __builtin_copysignf(x, y);
}
