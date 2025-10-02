#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "entity/components/renderer/bounds.h"
#include <stdlib.h>

void Bounds_Setup(Bounds* bounds, V3 position, V2 dimensions){
    bounds->position = position;
    bounds->dimensions = dimensions;
}