#include "utilities/math/v2.h"
#include "utilities/math/stupid_math.h"
#include <math.h>

const V2 V2_ONE = {1.0, 1.0};
const V2 V2_HALF = {0.5, 0.5};
const V2 V2_ZERO = {0.0, 0.0};

V2 V2_SCALE(V2 *a, float b)
{
    return (V2){a->x * b, a->y * b};
}
V2 V2_DIV(V2 *a, V2 *b)
{
    return (V2){a->x / b->x, a->y / b->y};
}
V2 V2_SUB(V2 *a, V2 *b)
{
    return (V2){a->x - b->x, a->y - b->y};
}
V2 V2_SIZE(V2 *a, V2 *b)
{
    float width = b->x - a->x;
    float height = b->y - a->y;
    width = width < 0 ? -width : width;
    height = height < 0 ? -height : height;
    return (V2){width, height};
}

inline V2 V2_Rand(float magnitude)
{
    float angle = RandomFloat(0.0, 2 * M_PI);
    return (V2){cosf(angle) * magnitude, sinf(angle) * magnitude};
}

/// @return A unit vector (magnitude of 1) in the direction of (x, y)
inline V2 V2_Direction(float x, float y){
    float length = sqrtf(x * x + y * y);
    if (length == 0){
        return V2_ZERO;
    }
    return (V2){x / length, y / length};
}

V2 V2_NORM(V2 a)
{
    float length = sqrtf(a.x * a.x + a.y * a.y);
    if (length == 0)
    {
        a.x = 0;
        a.y = 0;
    }
    else
    {
        a.x /= length;
        a.y /= length;
    }
    return a;
}

V2 V2_CENTER(V2 *a, V2 *b)
{
    return (V2){(a->x + b->x) * 0.5, (a->y + b->y) * 0.5};
}

float V2_DOT(V2 *a, V2 *b)
{
    return a->x * b->x + a->y * b->y;
}

float V2_INT_DOT(V2_INT *a, V2_INT *b)
{
    return a->x * b->x + a->y * b->y;
}

V2 V2_INT_SUB(V2_INT *a, V2_INT *b)
{
    return (V2){(float)(a->x - b->x), (float)(a->y - b->y)};
}

float V2_MAGNITUDE(V2 *a)
{
    return sqrtf(a->x * a->x + a->y * a->y);
}

V2 V2_MUL(V2 *a, V2 *b)
{
    return (V2){a->x * b->x, a->y * b->y};
}

float V2_INT_MAGNITUDE(V2_INT *a)
{
    return sqrtf(a->x * a->x + a->y * a->y);
}