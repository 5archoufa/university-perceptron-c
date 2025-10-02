#ifndef BOUNDS_H
#define BOUNDS_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"

typedef struct Bounds{
    V3 position;
    V2 dimensions;
} Bounds;

void Bounds_Setup(Bounds* bounds, V3 position, V2 dimensions);

#endif