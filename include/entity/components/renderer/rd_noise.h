#ifndef RD_NOISE_H
#define RD_NOISE_H

#include "utilities/noise/noise.h"
#include "entity/components/renderer/renderer.h"

typedef struct RD_Noise RD_Noise;
struct RD_Noise
{
    EC_Renderer *renderer;
    Noise *noise;
};

EC_Renderer *Prefab_Noise(Entity *parent, V3 position, float rotation, V2 scale, V2 pivot, Noise *noise);
EC_Renderer *RD_Noise_CreateWithRenderer(Entity *entity, Noise *noise);

#endif