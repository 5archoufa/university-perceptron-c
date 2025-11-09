#include "entity/components/ec_planet/ec_planet.h"
#include "entity/entity.h"
#include "entity/components/lighting/ls_directional.h"
#include <stdint.h>
#include "perceptron.h"

void EC_Planet_Free(Component *component)
{
    EC_Planet *ec_planet = component->self;
    free(ec_planet);
}

inline static void EC_Planet_UpdateRotation(EC_Planet *ec_planet)
{
    // Get current time
    SimulatedDateTime *dateTime = World_GetDateTime();
    float timeOfDay = dateTime->normalizedHours; // 0.5 => noon, 0.0 and 1.0 => midnight
    
    // Sun should rotate 180 degrees across the sky throughout the day
    // 0.0 (midnight) = -90° (below horizon in east)
    // 0.5 (noon) = +90° (zenith)
    // 1.0 (midnight) = +270° (below horizon in west)
    float sunAngle = (timeOfDay * 360.0f) - 90.0f;
    
    // Calculate sun altitude for intensity (0° = horizon, 90° = zenith, negative = below horizon)
    float sunAltitude = sunAngle;
    if (sunAltitude > 180.0f) sunAltitude -= 360.0f; // Normalize to -180 to 180
    if (sunAltitude > 90.0f) sunAltitude = 180.0f - sunAltitude; // Mirror above 90°
    
    // Calculate intensity based on sun altitude
    // Peak at noon (90°), fade to 0 at horizon (0°) and below
    float intensity = fmaxf(0.0f, sinf(sunAltitude * M_PI / 180.0f));
    
    // Optional: Add some ambient light even at night
    float minIntensity = 0.1f; // Moonlight/starlight
    intensity = minIntensity + intensity * (1.0f - minIntensity);
    
    // Update directional light rotation
    Quaternion rotation = Quat_FromEuler((V3){sunAngle, -45.0f, 0});
    T_LRot_Set(&ec_planet->ec_light->component->entity->transform, rotation);
    
    ec_planet->ls_directionalLight->intensity = intensity;
}

static void EC_Planet_Update(Component *component)
{
    EC_Planet *ec_planet = component->self;
    EC_Planet_UpdateRotation(ec_planet);
}

static EC_Planet *EC_Planet_Create(Entity *entity, LS_Directional *ls_directionalLight)
{
    EC_Planet *ec_planet = malloc(sizeof(EC_Planet));
    ec_planet->ec_light = ls_directionalLight->ec_light;
    ec_planet->ls_directionalLight = ls_directionalLight;
    // Component
    ec_planet->component = Component_Create(ec_planet, entity, EC_T_PLANET, EC_Planet_Free, NULL, NULL, EC_Planet_Update, NULL, NULL);
    return ec_planet;
}

EC_Planet *Prefab_Planet(Entity *parent, uint32_t color, float intensity)
{
    // Calculate rotation based off time of day
    Entity *entity = Entity_Create(parent, false, "Planet", TS_WORLD, (V3){0, 100, 0}, QUATERNION_IDENTITY, V3_ONE);
    // Directional Light
    EC_Light *ec_light = Prefab_DirectionalLight(entity, TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, V3_ONE, intensity, color);
    EC_Planet *ec_planet = EC_Planet_Create(entity, (LS_Directional *)ec_light->lightSource);
    // Initial rotation update
    EC_Planet_UpdateRotation(ec_planet);
    return ec_planet;
}