#include <math.h>

float distance2d(float x1, float y1, float x2, float y2) {
    float dist;
    dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

    return dist;
}

float distance3d (float x1, float y1, float z1, float x2, float y2, float z2)
{
	float dist;
	dist = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));

	return dist;
}
