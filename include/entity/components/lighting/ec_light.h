#ifndef EC_LIGHT_H
#define EC_LIGHT_H

#include "entity/entity.h"

typedef enum {
    LS_T_DIRECTIONAL,
    LS_T_POINT
} LS_Type;

typedef struct EC_Light
{
    Component *component;
    LS_Type type;
    void *lightSource;
    void (*ls_Free)(struct EC_Light *ec_light);
} EC_Light;

EC_Light *EC_Light_Create(Entity *entity, LS_Type type, void *lightSource, void (*ls_Free)(struct EC_Light *ec_light));

#endif // EC_LIGHT_H