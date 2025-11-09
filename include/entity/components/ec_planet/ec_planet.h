#ifndef EC_PLANET_H
#define EC_PLANET_H

#include "entity/entity.h"
#include "entity/components/lighting/ls_directional.h"
#include "entity/components/lighting/ec_light.h"
#include <stdint.h>

typedef struct EC_Planet EC_Planet;

struct EC_Planet
{
    Component *component;
    EC_Light *ec_light;
    LS_Directional *ls_directionalLight;
};

EC_Planet *Prefab_Planet(Entity *parent, uint32_t color, float intensity);

#endif