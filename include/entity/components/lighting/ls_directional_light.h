#ifndef LS_DIRECTIONAL_LIGHT_H
#define LS_DIRECTIONAL_LIGHT_H

#include "entity/entity.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

typedef struct LS_DirectionalLight LS_DirectionalLight;

struct LS_DirectionalLight
{
    EC_Light *ec_light;
    Component *component;
    float intensity;
    uint32_t color;
};

EC_Light *Prefab_DirectionalLight(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float intensity, uint32_t color);
#endif