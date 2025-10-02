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
#include "perceptron.h"
// static float timer = 0;

void Tick()
{
    // timer -= DeltaTime;
    // if (timer < 0)
    // {
    //     timer = ((float)rand() / (float)RAND_MAX) * 1.5f;

    //     Entity *E_circle = Entity_Create("Circle", (V3){((float)rand() / (float)RAND_MAX) * 3200, ((float)rand() / (float)RAND_MAX) * 1800, 0}, 0.0, V2_ZERO, V2_HALF);
    //     RD_Circle_CreateWithRenderer(E_circle, 0);
    // }
}

static bool CanEnter(StateMachine_State *previousState, int argCount, void **args)
{
    return true;
}

static void OnEnter(StateMachine_State *previousState, int argCount, void **args)
{
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