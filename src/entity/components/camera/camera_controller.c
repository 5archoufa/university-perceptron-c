#include "entity/components/camera/camera_controller.h"
#include "entity/components/camera/camera.h"
#include "input/input_manager.h"
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "logging/logger.h"
#include "perceptron.h"
#include "entity/transform.h"

static const int INPUT_BACKWARD = 0;
static const int INPUT_RIGHT = 1;
static const int INPUT_FORWARD = 2;
static const int INPUT_LEFT = 3;
static const int INPUT_UP = 4;
static const int INPUT_DOWN = 5;
static const int INPUT_TOGGLE_MODE = 6;
static const int INPUT_TOGGLE_RULERS = 7;

static LogConfig _logConfig = {"CameraController", LOG_LEVEL_WARN, LOG_COLOR_BLUE};

static void EC_CameraController_LateUpdate(Component *component)
{
    EC_CameraController *cameraController = component->self;
    InputContext *input = cameraController->inputContext;
    // Log(&_logConfig, "Camera position: (%.2f, %.2f, %.2f), eulerAngles: (%.2f, %.2f, %.2f)\n",
    if (input->keys[INPUT_RIGHT].isDown)
    {
        V3 right = EC_Right(component);
        T_LPos_Add(&component->entity->transform, V3_SCALE(right, cameraController->speed.x * DeltaTime));
    }
    if (input->keys[INPUT_LEFT].isDown)
    {
        V3 right = EC_Right(component);
        T_LPos_Add(&component->entity->transform, V3_SCALE(right, -cameraController->speed.x * DeltaTime));
    }
    if (input->keys[INPUT_FORWARD].isDown)
    {
        V3 forward = EC_Forward(component);
        T_LPos_Add(&component->entity->transform, V3_SCALE(forward, cameraController->speed.z * DeltaTime));
    }
    if (input->keys[INPUT_BACKWARD].isDown)
    {
        V3 forward = EC_Forward(component);
        T_LPos_Add(&component->entity->transform, V3_SCALE(forward, -cameraController->speed.z * DeltaTime));
    }
    if (input->keys[INPUT_UP].isDown)
    {
        T_LPos_Add(&component->entity->transform, V3_Up(cameraController->speed.y * DeltaTime));
    }
    if (input->keys[INPUT_DOWN].isDown)
    {
        T_LPos_Add(&component->entity->transform, V3_Up(-cameraController->speed.y * DeltaTime));
    }
    if (input->keys[INPUT_TOGGLE_MODE].isPressed)
    {
        Log(&_logConfig, "Toggling Camera Render Mode");
        cameraController->camera->renderMode = (cameraController->camera->renderMode + 1) % CAMERA_RENDER_MODE_COUNT;
    }
    if (input->keys[INPUT_TOGGLE_RULERS].isPressed)
    {
        cameraController->camera->areRulersVisible = !cameraController->camera->areRulersVisible;
        Log(&_logConfig, "Toggling Camera Rulers: %s", cameraController->camera->areRulersVisible ? "ON" : "OFF");
    }
}

static void EC_CameraController_Free(Component *component)
{
    EC_CameraController *cameraController = component->self;
    InputContext_Free(cameraController->inputContext);
    free(cameraController);
    LogFree(&_logConfig, "");
}

Component *EC_CameraController_Create(Entity *entity, EC_Camera *camera, V3 speed)
{
    EC_CameraController *cameraController = malloc(sizeof(EC_CameraController));
    int keys[] = {
        GLFW_KEY_DOWN,
        GLFW_KEY_RIGHT,
        GLFW_KEY_UP,
        GLFW_KEY_LEFT,
        GLFW_KEY_SPACE,
        GLFW_KEY_LEFT_SHIFT,
        GLFW_KEY_M,
        GLFW_KEY_R
    };
    cameraController->inputContext = InputContext_Create("CameraInputContext",
                                                         8,
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