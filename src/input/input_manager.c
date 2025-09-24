#include "input/input_manager.h"
#include "utilities/stack.h"
#include <X11/extensions/XInput2.h>

Stack *InputContexts = NULL;

void InputManager_Init()
{
    InputContexts = CreateStack();
}

void InputManager_Free()
{
    FreeStack(InputContexts);
}

void InputManager_PushContext(InputContext *inputContext)
{
    printf("input_manager:: PushContext(%s)\n", inputContext->name);
    InputContexts->Push(InputContexts, (void *)inputContext);
}

InputContext *InputContext_Create(char *name,
                                  int keyCount,
                                  int *keyIds,
                                  int buttonCount,
                                  int *buttonIds,
                                  int motionCount,
                                  int *motionIds)
{
    InputContext *inputContext = malloc(sizeof(InputContext));
    inputContext->name = name;
    // Keys
    inputContext->keyCount = keyCount;
    if (keyCount > 0)
    {
        inputContext->keys = malloc(sizeof(KeyState) * keyCount);
        for (int i = 0; i < keyCount; i++)
        {
            inputContext->keys[i] = (KeyState){keyIds[i], false, false, false};
        }
    }
    else
    {
        inputContext->keys = NULL;
    }
    // Buttons
    inputContext->buttonCount = buttonCount;
    if (buttonCount > 0)
    {
        inputContext->buttons = malloc(sizeof(ButtonState) * buttonCount);
        for (int i = 0; i < buttonCount; i++)
        {
            inputContext->buttons[i] = (ButtonState){buttonIds[i], false, false, false};
        }
    }
    else
    {
        inputContext->buttons = NULL;
    }
    // Motions
    inputContext->motionCount = motionCount;
    if (motionCount != 0)
    {
        inputContext->motions = malloc(sizeof(MotionState) * motionCount);
        for (int i = 0; i < motionCount; i++)
        {
            inputContext->motions[i] = (MotionState){motionIds[i], 0.0, 0.0};
        }
    }
    else
    {
        inputContext->motions = NULL;
    }
    InputManager_PushContext(inputContext);
    return inputContext;
}

void InputContext_Free(InputContext *inputContext)
{
    free(inputContext->keys);
    free(inputContext->buttons);
    free(inputContext->motions);
    free(inputContext);
}

static KeyState *GetkeyState(InputContext *inputContext, int keyId)
{
    for (int i = 0; i < inputContext->keyCount; i++)
    {
        if (inputContext->keys[i].id == keyId)
        {
            return &inputContext->keys[i];
        }
    }
    return NULL;
}
static ButtonState *GetButtonState(InputContext *inputContext, int buttonId)
{
    for (int i = 0; i < inputContext->buttonCount; i++)
    {
        if (inputContext->buttons[i].id == buttonId)
        {
            return &inputContext->buttons[i];
        }
    }
    return NULL;
}
static MotionState *GetMotionState(InputContext *inputContext, int motionId)
{
    for (int i = 0; i < inputContext->motionCount; i++)
    {
        if (inputContext->motions[i].id == motionId)
        {
            return &inputContext->motions[i];
        }
    }
    return NULL;
}

void InputManager_ResetInputs()
{
    for (int i = InputContexts->count - 1; i >= 0; i--)
    {
        InputContext *inputContext = (InputContext *)InputContexts->content[i];
        for (int i = 0; i < inputContext->keyCount; i++)
        {
            inputContext->keys[i].isPressed = false;
            inputContext->keys[i].isReleased = false;
        }
        for (int i = 0; i < inputContext->buttonCount; i++)
        {
            inputContext->buttons[i].isPressed = false;
            inputContext->buttons[i].isReleased = false;
        }
    }
}

void InputManager_HandleXEvent(XEvent *event)
{
    XGenericEventCookie *cookie = &event->xcookie;
    XIDeviceEvent *xievent = cookie->data;

    for (int i = InputContexts->count - 1; i >= 0; i--)
    {
        InputContext *inputContext = (InputContext *)InputContexts->content[i];
        // Handle new input event
        if (cookie->evtype == XI_KeyPress || cookie->evtype == XI_KeyRelease)
        {
            KeyState *key = GetkeyState(inputContext, xievent->detail);
            if (key != NULL)
            {
                key->isPressed = cookie->evtype == XI_KeyPress;
                key->isDown = key->isPressed;
                key->isReleased = !key->isPressed;
                printf("Saving value into state %d: keypress:%d, keydown:%d, keyrelease:%d\n", key->id, key->isPressed, key->isDown, key->isReleased);
            }
        }
        else if (cookie->evtype == XI_ButtonPress || cookie->evtype == XI_ButtonRelease)
        {
            ButtonState *button = GetButtonState(inputContext, xievent->detail);
            if (button != NULL)
            {
                button->isPressed = cookie->evtype == XI_ButtonPress;
                button->isDown = button->isPressed;
                button->isReleased = !button->isPressed;
            }
        }
        else if (cookie->evtype == XI_Motion)
        {
            for (int i = 0; i < inputContext->motionCount; i++)
            {
                inputContext->motions[i].x = xievent->event_x;
                inputContext->motions[i].y = xievent->event_y;
            }
        }

        if (inputContext->blockEvents)
        {
            break;
        }
    }
}