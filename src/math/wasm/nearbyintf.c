#include <math.h>

__attribute__((const)) float nearbyintf(float x)
{
	return __builtin_nearbyintf(x);
}
