#ifndef EC_WATER_H
#define EC_WATER_H

#include "entity/entity.h"
#include "entity/transform.h"
#include "utilities/noise/noise.h"

typedef struct
{
    Component *component;
    float waveSpeed;
    float waveHeight;
    float waveFrequency;
    EC_Renderer3D *ec_renderer3d_water;
    Noise* noise;
} EC_Water;

EC_Water *Prefab_Water(Entity *e_parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale, size_t modifiers_size, NoiseModifier* modifiers);

#endif