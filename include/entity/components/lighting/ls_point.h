#ifndef LS_POINT_H
#define LS_POINT_H

#include "entity/entity.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

// OpenGL
#define GLFW_INCLUDE_NONE
#include <cglm/cglm.h>


typedef struct LS_Point LS_Point;

struct LS_Point
{
    Component *component;
    EC_Light *ec_light;
    float intensity;
    float range;
    // Color
    uint32_t color;
    vec3 color_openGL;
};

// -------------------------
// Setters
// -------------------------
void LS_Point_SetColor(LS_Point *ls_point, uint32_t color);
// -------------------------
// Prefabs
// -------------------------
EC_Light *Prefab_PointLight(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float intensity, float range, uint32_t color);
#endif