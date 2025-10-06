#include "entity/components/ec_planet/ec_planet.h"
#include "entity/entity.h"
#include "entity/components/lighting/ls_directional_light.h"
#include <stdint.h>
#include "perceptron.h"

void EC_Planet_Free(Component *component)
{
    EC_Planet *ec_planet = component->self;
    free(ec_planet);
}

static void EC_Planet_Update(Component *component)
{
    EC_Planet *ec_planet = component->self;
    // Rotate Planet
    V3 addition = V3_SCALE(ec_planet->rotationSpeed, DeltaTime);
    T_LRot_Add(&component->entity->transform, addition);
}

static EC_Planet *EC_Planet_Create(Entity *entity, V3 rotationSpeed, LS_DirectionalLight *ls_directionalLight)
{
    EC_Planet *ec_planet = malloc(sizeof(EC_Planet));
    ec_planet->rotationSpeed = rotationSpeed;
    ec_planet->ec_light = ls_directionalLight->ec_light;
    ec_planet->ls_directionalLight = ls_directionalLight;
    // Component
    ec_planet->component = Component_Create(ec_planet, entity, EC_T_PLANET, EC_Planet_Free, NULL, NULL, EC_Planet_Update, NULL, NULL);
    return ec_planet;
}

EC_Planet *Prefab_Planet(Entity *parent, V3 rotationSpeed, uint32_t color, float intensity)
{
    Entity *entity = Entity_Create(parent, "Planet", TS_WORLD, (V3){0, 100, 0}, QUATERNION_IDENTITY, V3_ONE);
    // Create cube
    Entity *e_cube = Prefab_Cube(entity, TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, V3_ONE, 50, NULL)->component->entity;
    // Directional Light
    EC_Light *ec_light = Prefab_DirectionalLight(entity, TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, V3_ONE, intensity, color);
    EC_Planet *ec_planet = EC_Planet_Create(entity, rotationSpeed, (LS_DirectionalLight *)ec_light->lightSource);
    return ec_planet;
}