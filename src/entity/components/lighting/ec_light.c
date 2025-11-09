#include "entity/components/lighting/ec_light.h"

static void EC_Light_Free(Component* component) {
    EC_Light* ec_light = component->self;
    if(!ec_light){
        return;
    }
    World_Light_Remove(ec_light);
    ec_light->ls_Free(ec_light);
    free(ec_light);
}

EC_Light* EC_Light_Create(Entity *entity, LS_Type type, void* lightSource, void (*LS_Free)(struct EC_Light* ec_light)) {
    EC_Light* ec_light = malloc(sizeof(EC_Light));
    // Light source
    ec_light->lightSource = lightSource;
    ec_light->type = type;
    ec_light->ls_Free = LS_Free;
    // Component
    ec_light->component = Component_Create(ec_light, entity, EC_T_LIGHT, EC_Light_Free, NULL, NULL, NULL, NULL, NULL);
    printf("Created EC_Light %p for Entity %s with component %p\n", ec_light, entity->name, ec_light->component);
    // Register light in world
    World_Light_Add(ec_light);
    return ec_light;
}