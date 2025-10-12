#ifndef AABB_H
#define AABB_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"

typedef struct AABB{
    V3 min;
    V3 max;
    V3 size;
} AABB;

void AABB_Setup(AABB* bounds, V3 start, V3 end, V3 size);
#endif