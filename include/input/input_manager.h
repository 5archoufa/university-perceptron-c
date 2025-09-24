#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utilities/stack.h"

extern Stack *InputContexts;



typedef struct
{
    int id;
    bool isPressed;
    bool isReleased;
    bool isDown;
} KeyState;

typedef struct
{
    int id;
    bool isPressed;
    bool isReleased;
    bool isDown;
} ButtonState;

typedef struct
{
    int id;
    float x, y;
} MotionState;

typedef struct InputContext
{
    char *name;
    // Inputs States
    int keyCount;
    KeyState *keys;
    int buttonCount;
    ButtonState *buttons;
    int motionCount;
    MotionState *motions;
    /// Prevents input events from going further in the stack.
    bool blockEvents;
} InputContext;

void InputManager_HandleXEvent(XEvent *event);
InputContext *InputContext_Create(char *name,
                                  int keyCount,
                                  int *keyIds,
                                  int buttonCount,
                                  int *buttonIds,
                                  int motionCount,
                                  int *motionIds);
void InputContext_Free(InputContext *inputContext);
void InputManager_ResetInputs();
void InputManager_Free();
void InputManager_Init();
void InputManager_PushContext(InputContext *inputContext);
#endif