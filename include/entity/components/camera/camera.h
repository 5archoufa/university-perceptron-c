#ifndef EC_CAMERA_H
#define EC_CAMERA_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"
#include <stdbool.h>
#include "ui/window.h"
#include "entity/entity.h"
#include "entity/components/renderer/bounds.h"
#include <X11/Xlib.h>
#include "world/world.h"

typedef struct EC_Camera EC_Camera;

struct EC_Camera{
    Component* component;
    V2 viewport;
    World* world;
    XImage* image;
    float pixelsPerMeter;
    V3* position;
};

EC_Camera *EC_Camera_Create(Entity* entity, XImage* image, V2 viewport);
void EC_Camera_Free(Component* component);
V2_INT Camera_WorldToScreen_V2(EC_Camera* EC_camera, V2* position);
V2_INT Camera_WorldToScreen_V3(EC_Camera* EC_camera, V3* position);
#endif