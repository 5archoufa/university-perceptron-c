#include "utilities/math/v3.h"
#include "shape.h"

typedef struct
{
    V3 center;
    float r;
} Circle;

Shape *CreateCircle(V3 center, float radius);
Pixel *DrawCircle(void *self);