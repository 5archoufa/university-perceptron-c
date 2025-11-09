#include "entity/components/lighting/ls_point.h"
#include "entity/entity.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

static void LS_PointLight_Free(EC_Light *light)
{
    LS_Point *ls_light = light->lightSource;
    free(ls_light);
}

static LS_Point *LS_PointLight_Create(float intensity, float range, uint32_t color)
{
    LS_Point *ls_point = malloc(sizeof(LS_Point));
    ls_point->intensity = intensity;
    LS_Point_SetColor(ls_point, color);
    return ls_point;
}

void LS_Point_SetColor(LS_Point *ls_point, uint32_t color)
{
    ls_point->color = color;
    // Extract RGB components as floats [0,1]
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    glm_vec3((vec3){r, g, b}, ls_point->color_openGL);
}

EC_Light *Prefab_PointLight(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float intensity, float range, uint32_t color)
{
    Entity *e_pointLight = Entity_Create(parent, false, "Point Light", TS, position, rotation, scale);
    LS_Point *ls_pointLight = LS_PointLight_Create(intensity, range, color);
    EC_Light *ec_light = EC_Light_Create(e_pointLight, LS_T_POINT, ls_pointLight, LS_PointLight_Free);
    return ec_light;
}