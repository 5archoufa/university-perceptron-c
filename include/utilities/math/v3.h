#ifndef V3_H
#define V3_H
#include "utilities/math/v2.h"

typedef struct V3 V3;

extern const V3 V3_ONE;
extern const V3 V3_HALF;
extern const V3 V3_ZERO;

struct V3{
    float x, y, z;
};

V3 V3_ADD(const V3* a, const V3* b);
void V3_ADD_FIRST(V3 *a, const V3 *b);
V3 V3_SUB(const V3* a, const V3* b);
V3 V3_MUL(const V3* a, const V3* b);
V3 V3_NEG(const V3* a);
V3 V3_ADD_V2(const V3 *a, const V2 *b);
#endif