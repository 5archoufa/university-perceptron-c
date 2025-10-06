#ifndef BOUNDS_H
#define BOUNDS_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"

typedef struct Bounds{
    V3 start;
    V3 end;
    V3 size;
} Bounds;

void Bounds_Setup(Bounds* bounds, V3 start, V3 end, V3 size);
#endif