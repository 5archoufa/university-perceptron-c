#include <stdlib.h>
#include <time.h>
#include "utilities/math/stupid_math.h"
#include "utilities/math/v2.h"
#include <math.h>
#include <stdbool.h>

static const float _PI_OVER_180 = M_PI / 180.0f;
static const float _180_OVER_PI = 180.0f / M_PI;

inline float RandomFloat(float min, float max)
{
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

inline float Degrees(float radians)
{
    return radians * _180_OVER_PI;
}
inline float Radians(float degrees)
{
    return degrees * _PI_OVER_180;
}
inline float Clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
inline int ClampInt(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
inline bool IsBetween(float value, float min, float max){
    return value >= min && value <= max;
}