#include <stdio.h>
#include "utilities/math/v3.h"
#include <math.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <cglm/cglm.h>

const V3 V3_ZERO = {0.0, 0.0, 0.0};
const V3 V3_HALF = {0.5, 0.5, 0.5};
const V3 V3_ONE = {1.0, 1.0, 1.0};

const vec3 vec3_ZERO = {0.0f, 0.0f, 0.0f};
const vec3 vec3_HALF = {0.5f, 0.5f, 0.5f};
const vec3 vec3_ONE = {1.0f, 1.0f, 1.0f};
const vec3 vec3_UP = {0.0f, 1.0f, 0.0f};
const vec3 vec3_RIGHT = {1.0f, 0.0f, 0.0f};
const vec3 vec3_FORWARD = {0.0f, 0.0f, 1.0f};
const vec3 vec3_DOWN = {0.0f, -1.0f, 0.0f};
const vec3 vec3_LEFT = {-1.0f, 0.0f, 0.0f};
const vec3 vec3_BACK = {0.0f, 0.0f, -1.0f};

inline V3 V3_ADD(const V3 a, const V3 b) { return (V3){a.x + b.x, a.y + b.y, a.z + b.z}; }
inline void V3_ADD_FIRST(V3 a, const V3 *b)
{
    a.x += b->x;
    a.y += b->y;
    a.z += b->z;
}
inline void V3_ADD_V2_FIRST(V3 *a, const V2 *b)
{
    a->x += b->x;
    a->y += b->y;
}
inline V3 V3_SUB(const V3 a, const V3 b) { return (V3){a.x - b.x, a.y - b.y, a.z - b.z}; }
inline V3 V3_MUL(const V3 a, const V3 b) { return (V3){a.x * b.x, a.y * b.y, a.z * b.z}; }
inline V3 V3_SCALE(const V3 a, const float scalar) { return (V3){a.x * scalar, a.y * scalar, a.z * scalar}; }
inline V3 V3_CROSS(const V3 a, const V3 b) { return (V3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
inline V3 V3_DIV(const V3 a, const V3 b) { return (V3){a.x / b.x, a.y / b.y, a.z / b.z}; }
inline V3 V3_NEG(const V3 a) { return (V3){-a.x, -a.y, -a.z}; }
inline V3 V3_ADD_V2(const V3 a, const V2 b) { return (V3){a.x + b.x, a.y + b.y, a.z}; }
inline float V3_DOT(const V3 a, const V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
V3 V3_Up(float y)
{
    return (V3){0.0, y, 0.0};
}
V3 V3_Right(float x)
{
    return (V3){x, 0.0, 0.0};
}
V3 V3_Forward(float z)
{
    return (V3){0.0, 0.0, z};
}
inline V3 V3_MIN(V3 a, V3 b)
{
    return (V3){
        .x = (a.x < b.x) ? a.x : b.x,
        .y = (a.y < b.y) ? a.y : b.y,
        .z = (a.z < b.z) ? a.z : b.z};
}
inline V3 V3_ABS(const V3 a)
{
    return (V3){
        .x = (a.x < 0) ? -a.x : a.x,
        .y = (a.y < 0) ? -a.y : a.y,
        .z = (a.z < 0) ? -a.z : a.z};
}

inline V3 V3_MAX(V3 a, V3 b)
{
    return (V3){
        .x = (a.x > b.x) ? a.x : b.x,
        .y = (a.y > b.y) ? a.y : b.y,
        .z = (a.z > b.z) ? a.z : b.z};
}

inline float V3_MAGNITUDE(const V3 a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline V3 V3_NORM(const V3 a)
{
    V3 result = a;
    float length = sqrtf(result.x * result.x + result.y * result.y + result.z * result.z);
    if (length != 0)
    {
        result.x /= length;
        result.y /= length;
        result.z /= length;
    }
    return result;
}

inline bool V3_EQUALS(const V3 a, const V3 b)
{
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

inline V3 V3_SIZE(const V3 a, const V3 b)
{
    float width = b.x - a.x;
    float height = b.y - a.y;
    float depth = b.z - a.z;
    width = width < 0 ? -width : width;
    height = height < 0 ? -height : height;
    depth = depth < 0 ? -depth : depth;
    return (V3){width, height, depth};
}

inline V3 V3_CENTER(const V3 a, const V3 b)
{
    return (V3){(a.x + b.x) * 0.5, (a.y + b.y) * 0.5, (a.z + b.z) * 0.5};
}