#ifndef RD_ISLAND_H
#define RD_ISLAND_H

#include "entity/components/renderer/renderer.h"
#include "utilities/noise/noise.h"

typedef struct
{
    EC_Renderer *ec_renderer;
    Noise *noise;
    float unitsPerCell;
} RD_Island;

EC_Renderer* RD_Island_CreateWithRenderer(Entity* entity, int width, int height, float interpolation, float unitsPerCell);

#endif