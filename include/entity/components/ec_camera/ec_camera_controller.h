#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "entity/entity.h"
#include "entity/components/ec_camera/ec_camera.h"
#include "input/input_manager.h"

typedef struct EC_CameraController{
    Component* component;
    EC_Camera* camera;
    V3 speed;
    InputListener *inputListener;
} EC_CameraController;

Component *EC_CameraController_Create(Entity* entity, EC_Camera *camera, V3 speed);

#endif