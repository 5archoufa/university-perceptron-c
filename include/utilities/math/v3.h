#ifndef V3_H
#define V3_H

#include "utilities/math/v2.h"
// OpenGL
#include <cglm/cglm.h>

typedef struct V3 V3;

// V3
extern const V3 V3_ONE;
extern const V3 V3_HALF;
extern const V3 V3_ZERO;
// vec3
extern const vec3 vec3_UP;
extern const vec3 vec3_HALF;
extern const vec3 vec3_ONE;
extern const vec3 vec3_ZERO;
extern const vec3 vec3_RIGHT;
extern const vec3 vec3_FORWARD;
extern const vec3 vec3_DOWN;
extern const vec3 vec3_LEFT;
extern const vec3 vec3_BACK;

struct V3{
    float x, y, z;
};

V3 V3_ADD(const V3 a, const V3 b);
void V3_ADD_FIRST(V3 a, const V3 *b);
void V3_ADD_V2_FIRST(V3 *a, const V2 *b);
V3 V3_SUB(const V3 a, const V3 b);
V3 V3_MUL(const V3 a, const V3 b);
V3 V3_CROSS(const V3 a, const V3 b);
V3 V3_DIV(const V3 a, const V3 b);
V3 V3_NEG(const V3 a);
V3 V3_ADD_V2(const V3 a, const V2 b);
float V3_DOT(const V3 a, const V3 b);
V3 V3_SCALE(const V3 a, const float scalar);
V3 V3_SIZE(const V3 a, const V3 b);
V3 V3_NORM(const V3 a);
V3 V3_CENTER(const V3 a, const V3 b);
V3 V3_Up(float y);
V3 V3_Right(float x);
float V3_MAGNITUDE(const V3 a);
V3 V3_Forward(float z);
V3 V3_MAX(V3 a, V3 b);
V3 V3_MIN(V3 a, V3 b);
V3 V3_ABS(const V3 a);
bool V3_EQUALS(const V3 a, const V3 b);
#endif