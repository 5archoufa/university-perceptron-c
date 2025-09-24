#ifndef EC_CAMERA_H
#define EC_CAMERA_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"
#include <stdbool.h>
#include "ui/window.h"
#include "entity/entity.h"
#include "entity/components/renderer/bounds.h"
#include <X11/Xlib.h>

typedef struct{
    Component* component;
    V2 viewport;
    XImage* image;
} EC_Camera;

EC_Camera *EC_Camera_Create(Entity* entity, XImage* image, V2 viewport);
void EC_Camera_Free(Component* component);
void RenderCameraDebug(MyWindow* myWindow, EC_Camera* camera);
#endif