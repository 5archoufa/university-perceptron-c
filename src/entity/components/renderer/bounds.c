#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "entity/components/renderer/bounds.h"
#include <stdlib.h>

void Bounds_Setup(Bounds* bounds, V3 start, V3 end, V3 size){
    bounds->start = start;
    bounds->end = end;
    bounds->size = size;
}