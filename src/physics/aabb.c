#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "physics/aabb.h"
#include <stdlib.h>

void AABB_Setup(AABB* aabb, V3 min, V3 max, V3 size){
    aabb->min = min;
    aabb->max = max;
    aabb->size = size;
}