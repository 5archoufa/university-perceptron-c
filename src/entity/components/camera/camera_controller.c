#include "entity/components/camera/camera_controller.h"
#include "entity/components/camera/camera.h"
#include "input/input_manager.h"
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "logging/logger.h"
#include "perceptron.h"

static const int INPUT_UP = 2;
static const int INPUT_DOWN = 0;
static const int INPUT_LEFT = 3;
static const int INPUT_RIGHT = 1;

static LogConfig _logConfig = {"CameraController", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

static void EC_CameraController_LateUpdate(Component *component)
{
    EC_CameraController *cameraController = component->self;
    InputContext *input = cameraController->inputContext;
    if (input->keys[INPUT_RIGHT].isDown)
    {
        Entity_AddPos(component->entity, (V3){cameraController->speed.x * DeltaTime, 0.0, 0.0} );
    }
    if (input->keys[INPUT_LEFT].isDown)
    {
        Entity_AddPos(component->entity, (V3){-cameraController->speed.x * DeltaTime, 0.0, 0.0});
    }
    if (input->keys[INPUT_UP].isDown)
    {
        Entity_AddPos(component->entity, (V3){0.0, cameraController->speed.y * DeltaTime, 0.0});
    }
    if (input->keys[INPUT_DOWN].isDown)
    {
        Entity_AddPos(component->entity, (V3){0.0, -cameraController->speed.y * DeltaTime, 0.0});
    }
}

static void EC_CameraController_Free(Component *component)
{
    EC_CameraController *cameraController = component->self;
    InputContext_Free(cameraController->inputContext);
    free(cameraController);
    LogFree(&_logConfig, "");
}

Component *EC_CameraController_Create(Entity *entity, EC_Camera *camera, V2 speed)
{
    EC_CameraController *cameraController = malloc(sizeof(EC_CameraController));
    // Input Context
    int keys[] = {
        XKeysymToKeycode(MainWindow->display, XK_Up),
        XKeysymToKeycode(MainWindow->display, XK_Right),
        XKeysymToKeycode(MainWindow->display, XK_Down),
        XKeysymToKeycode(MainWindow->display, XK_Left)
    };
    cameraController->inputContext = InputContext_Create("CameraInputContext",
                                                         4,
                                                         keys,
                                                         0,
                                                         NULL,
                                                         0,
                                                         NULL);
    // Speed
    cameraController->speed = speed;
    // Camera
    
    cameraController->camera = camera;
    // Component
    Component *component = Component_Create(cameraController, entity, EC_T_CAMERA_CONTROLLER, EC_CameraController_Free, NULL, NULL, NULL, EC_CameraController_LateUpdate, NULL);
    cameraController->component = component;
    LogCreate(&_logConfig, "");
    return component;
}