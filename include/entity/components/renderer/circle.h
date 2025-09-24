#ifndef EC_CIRCLE_H
#define EC_CIRCLE_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"
#include "entity/entity.h"
#include "entity/components/renderer/renderer.h"

typedef struct
{
    EC_Renderer* renderer;
    float radius;
} RD_Circle;

EC_Renderer* RD_Circle_CreateWithRenderer(Entity* entity, float radius);
#endif