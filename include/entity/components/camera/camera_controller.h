#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "entity/entity.h"
#include "entity/components/camera/camera.h"
#include "input/input_manager.h"

typedef struct EC_CameraController{
    Component* component;
    EC_Camera* camera;
    V2 speed;
    InputContext* inputContext;
} EC_CameraController;

Component *EC_CameraController_Create(Entity* entity, EC_Camera *camera, V2 speed);

#endif