#include <stdlib.h>
#include <time.h>
#include "utilities/math/stupid_math.h"
#include "utilities/math/v2.h"
#include <math.h>

float RandomFloat(float min, float max)
{
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

V2 RandomV2(float magnitude)
{
    float angle = RandomFloat(0.0, 2 * M_PI);
    return (V2){cosf(angle) * magnitude, sinf(angle) * magnitude};
}