#include "entity/components/ec_camera/ec_camera_controller.h"
#include "perceptron.h"
#include "game/game.h"
// Camera
#include "entity/components/ec_camera/ec_camera.h"
// Input
#include "input/input_manager.h"
// C
#include <stdio.h>
// Logging
#include "logging/logger.h"
// Entity
#include "entity/transform.h"

static KeyState *KEY_BACKWARD = NULL;
static KeyState *KEY_RIGHT = NULL;
static KeyState *KEY_FORWARD = NULL;
static KeyState *KEY_LEFT = NULL;
static KeyState *KEY_UP = NULL;
static KeyState *KEY_DOWN = NULL;
static KeyState *KEY_MODE_TOGGLE_WIREFRAME = NULL;
static KeyState *KEY_MODE_TOGGLE_SOLID = NULL;
static KeyState *KEY_MODE_TOGGLE_COLLIDERS = NULL;
static MotionState *MOUSE_MOTION = NULL;

static LogConfig _logConfig = {"CameraController", LOG_LEVEL_WARN, LOG_COLOR_BLUE};

static void EC_CameraController_LateUpdate(Component *component)
{
    EC_CameraController *cameraController = component->self;
    InputListener *inputListener = cameraController->inputListener;
    V3 input_move = {
        KEY_RIGHT->isDown - KEY_LEFT->isDown,
        KEY_UP->isDown - KEY_DOWN->isDown,
        KEY_FORWARD->isDown - KEY_BACKWARD->isDown,
    };
    if (input_move.x != 0 || input_move.y != 0 || input_move.z != 0)
    {
        input_move = V3_NORM(input_move);
        V3 right = EC_Right(component);
        V3 forward = EC_Forward(component);
        forward.y = 0;
        forward = V3_NORM(forward);
        right = V3_SCALE(right, input_move.x * cameraController->speed.x * DeltaTime);
        forward = V3_SCALE(forward, input_move.z * cameraController->speed.y * DeltaTime);
        V3 up = V3_SCALE(WORLD_UP, input_move.y * cameraController->speed.z * DeltaTime);
        T_LPos_Add(&component->entity->transform, V3_ADD(V3_ADD(right, forward), up));
    }
    if (KEY_MODE_TOGGLE_WIREFRAME->isPressed)
    {
        Log(&_logConfig, "Toggling Camera Wireframe Mode");
        cameraController->camera->renderWireframe = !cameraController->camera->renderWireframe;
    }
    if (KEY_MODE_TOGGLE_SOLID->isPressed)
    {
        Log(&_logConfig, "Toggling Camera Solid Mode");
        cameraController->camera->renderSolid = !cameraController->camera->renderSolid;
    }
    if (KEY_MODE_TOGGLE_COLLIDERS->isPressed)
    {
#ifndef DEBUG_COLLIDERS
        LogWarning(&_logConfig, "The flag DEBUG_COLLIDERS is not defined.");
#else
        Log(&_logConfig, "Toggling Camera Colliders");
        cameraController->camera->renderColliders = !cameraController->camera->renderColliders;
#endif
    }
    // ============ Looking Around ============ //
    float mouseSensitivity = 50.0f;
    float yawDelta = MOUSE_MOTION->delta.x * mouseSensitivity * DeltaTime;
    float pitchDelta = MOUSE_MOTION->delta.y * mouseSensitivity * DeltaTime;

    // Only rotate if there's actual mouse movement
    if (yawDelta != 0.0f || pitchDelta != 0.0f)
    {
        // Get current camera rotation
        Quaternion currentRot = T_LRot(&component->entity->transform);

        // Apply yaw rotation around world Y-axis
        Quaternion yawRotation = Quat_FromAxisAngle((V3){0.0f, 1.0f, 0.0f}, yawDelta);
        Quaternion newRotation = Quat_Mul(yawRotation, currentRot);

        // Get the right vector after yaw rotation
        V3 rightVector = Quat_RotateV3(newRotation, (V3){1.0f, 0.0f, 0.0f});

        // === CORRECTED PITCH CLAMPING ===
        // Calculate current pitch by checking the forward vector's Y component
        V3 forward = Quat_RotateV3(newRotation, (V3){0.0f, 0.0f, 1.0f});
        float currentPitch = -asinf(forward.y) * (180.0f / 3.14159265359f); // Negated for correct direction

        // Clamp the new pitch
        float newPitch = currentPitch + pitchDelta;
        if (newPitch > 89.0f)
        {
            pitchDelta = -89.0f + currentPitch;
        }
        else if (newPitch < -89.0f)
        {
            pitchDelta = 89.0f + currentPitch;
        }

        // Apply clamped pitch rotation
        Quaternion pitchRotation = Quat_FromAxisAngle(rightVector, pitchDelta);
        newRotation = Quat_Mul(pitchRotation, newRotation);

        // Normalize to prevent drift
        newRotation = Quat_Norm(newRotation);

        // Set the new rotation
        T_LRot_Set(&component->entity->transform, newRotation);
    }
}

static void EC_CameraController_Free(Component *component)
{
    EC_CameraController *cameraController = component->self;
    // Note: InputListener is owned by the InputContext and will be freed when context is freed
    // Do not call InputListener_Free here to avoid double-free
    free(cameraController);
    LogFree(&_logConfig, "");
}

Component *EC_CameraController_Create(Entity *entity, EC_Camera *camera, V3 speed)
{
    EC_CameraController *cameraController = malloc(sizeof(EC_CameraController));
    // Input Listener
    cameraController->inputListener = InputContext_AddListener(INPUT_CONTEXT_GAMEPLAY, "Camera Controller");
    InputMapping *mapping = InputListener_AddMapping(cameraController->inputListener, INPUT_CONTEXT_GAMEPLAY->id);
    KEY_BACKWARD = InputMapping_AddKey(mapping, GLFW_KEY_DOWN);
    KEY_RIGHT = InputMapping_AddKey(mapping, GLFW_KEY_RIGHT);
    KEY_FORWARD = InputMapping_AddKey(mapping, GLFW_KEY_UP);
    KEY_LEFT = InputMapping_AddKey(mapping, GLFW_KEY_LEFT);
    KEY_UP = InputMapping_AddKey(mapping, GLFW_KEY_SPACE);
    KEY_DOWN = InputMapping_AddKey(mapping, GLFW_KEY_LEFT_SHIFT);
    KEY_MODE_TOGGLE_WIREFRAME = InputMapping_AddKey(mapping, GLFW_KEY_I);
    KEY_MODE_TOGGLE_SOLID = InputMapping_AddKey(mapping, GLFW_KEY_O);
    KEY_MODE_TOGGLE_COLLIDERS = InputMapping_AddKey(mapping, GLFW_KEY_P);
    MOUSE_MOTION = InputMapping_AddMotion(mapping, 0);
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