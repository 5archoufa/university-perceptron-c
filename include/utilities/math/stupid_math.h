#ifndef STUPID_MATH_H
#define STUPID_MATH_H

#include "utilities/math/v2.h"
#include <stdbool.h>

float RandomFloat(float min, float max);
V2 V2_Rand(float magnitude);
float Degrees(float radians);
float Radians(float degrees);
float Clamp(float value, float min, float max);
int ClampInt(int value, int min, int max);
bool IsBetween(float value, float min, float max);

#endif