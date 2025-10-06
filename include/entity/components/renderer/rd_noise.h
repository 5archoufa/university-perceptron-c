#ifndef RD_NOISE_H
#define RD_NOISE_H

#include "utilities/noise/noise.h"
#include "entity/components/renderer/renderer.h"
#include "entity/transform.h"

typedef struct RD_Noise RD_Noise;
struct RD_Noise
{
    EC_Renderer *renderer;
    Noise *noise;
};

// EC_Renderer *EC_Renderer *Prefab_Noise(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, Noise *noise);
// EC_Renderer *RD_Noise_CreateWithRenderer(Entity *entity, Noise *noise);

#endif