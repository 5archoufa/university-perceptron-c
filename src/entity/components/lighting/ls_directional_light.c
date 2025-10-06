#include "entity/components/lighting/ls_directional_light.h"
#include "entity/entity.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

static void LS_DirectionalLight_Free(EC_Light *light)
{
    LS_DirectionalLight *ls_light = light->lightSource;
    free(ls_light);
}

static LS_DirectionalLight *LS_DirectionalLight_Create(float intensity, uint32_t color)
{
    LS_DirectionalLight *light = malloc(sizeof(LS_DirectionalLight));
    light->intensity = intensity;
    light->color = color;
    return light;
}

EC_Light *Prefab_DirectionalLight(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float intensity, uint32_t color)
{
    Entity *e_directionalLight = Entity_Create(parent, "Directional Light", TS, position, rotation, scale);
    LS_DirectionalLight *ls_directionalLight = LS_DirectionalLight_Create(intensity, color);
    EC_Light *ec_light = EC_Light_Create(e_directionalLight, LS_T_DIRECTIONAL, ls_directionalLight, LS_DirectionalLight_Free);
    return ec_light;
}