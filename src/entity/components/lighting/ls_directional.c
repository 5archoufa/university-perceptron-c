#include "entity/components/lighting/ls_directional.h"
#include "entity/entity.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

static void LS_Directional_Free(EC_Light *light)
{
    LS_Directional *ls_directional = light->lightSource;
    free(ls_directional);
}

static LS_Directional *LS_Directional_Create(float intensity, uint32_t color)
{
    LS_Directional *ls_directional = malloc(sizeof(LS_Directional));
    ls_directional->intensity = intensity;
    LS_Directional_SetColor(ls_directional, color);
    return ls_directional;
}

void LS_Directional_SetColor(LS_Directional *ls_directional, uint32_t color)
{
    ls_directional->color = color;
    // Extract RGB components as floats [0,1]
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    glm_vec3((vec3){r, g, b}, ls_directional->color_openGL);
}

EC_Light *Prefab_DirectionalLight(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float intensity, uint32_t color)
{
    Entity *e_directional = Entity_Create(parent, "Directional Light", TS, position, rotation, scale);
    LS_Directional *ls_directional = LS_Directional_Create(intensity, color);
    EC_Light *ec_light = EC_Light_Create(e_directional, LS_T_DIRECTIONAL, ls_directional, LS_Directional_Free);
    return ec_light;
}