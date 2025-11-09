#ifndef LS_DIRECTIONAL_H
#define LS_DIRECTIONAL_H

#include "entity/entity.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

typedef struct LS_Directional LS_Directional;

struct LS_Directional
{
    EC_Light *ec_light;
    float intensity;
    uint32_t color;
    vec3 color_openGL;
};

// -------------------------
// Setters
// -------------------------

void LS_Directional_SetColor(LS_Directional *ls_directional, uint32_t color);

// -------------------------
// Prefabs
// -------------------------

EC_Light *Prefab_DirectionalLight(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float intensity, uint32_t color);
#endif