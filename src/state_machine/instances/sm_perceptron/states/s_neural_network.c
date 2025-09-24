#include "state_machine/state_machine.h"
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include "utilities/math/v3.h"
#include "input/input_manager.h"
#include "entity/components/camera/camera.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/renderer/circle.h"
#include "entity/components/camera/camera_controller.h"

static Entity* E_camera = NULL;

void Tick(){
    
}

static bool CanEnter(StateMachine_State *previousState, int argCount, void **args)
{
    return true;
}

static void OnEnter(StateMachine_State *previousState, int argCount, void **args)
{
    // Camera
    E_camera = Entity_Create_WithoutParent("Main Camera", V3_ZERO, 0.0, V2_ONE, V2_HALF);
    EC_Camera *camera = EC_Camera_Create(E_camera, MainWindow->image,(V2){1600, 900});
    EC_CameraController_Create(E_camera, camera, (V2){1.0, 1.0});
    
    // Neuron
    Entity* E_circle = Entity_Create_WithoutParent("Circle", V3_ZERO, 0.0, V2_ZERO, V2_HALF);
    EC_Renderer* EC_circle_renderer = RD_Circle_CreateWithRenderer(E_circle, 100);
    
    // V3 center = {1000, 500, 0};
    // Shape *shape_circle = CreateCircle(center, 0.0, V2_HALF, 300);

    // center.x = 0;
    // center.y = 0;
    // Shape *shape_circle2 = CreateCircle(center, 0.0, V2_HALF, 200);

    // Shape *shapes[] = {shape_circle, shape_circle2};
    // int shapeCount = 2;
}

static bool CanExit(StateMachine_State *nextState, int argCount, void **args)
{
    return true;
}

static void OnExit(StateMachine_State *nextState, int argCount, void **args)
{
}

StateMachine_State *SMPerceptron_SNeuralNetwork_Create()
{
    StateMachine_State *state = StateMachine_State_Create("SNeuralNetwork", Tick, CanEnter, OnEnter, CanExit, OnExit);
    return state;
}

void SMPerceptron_SNeuralNetwork_Free(StateMachine_State *state)
{
    StateMachine_State_Free(state);
}