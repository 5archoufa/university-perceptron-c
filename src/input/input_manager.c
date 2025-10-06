#include "input/input_manager.h"
#include "utilities/stack.h"
#include "logging/logger.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Stack *InputContexts = NULL;
static LogConfig _logConfig = {"InputManager", LOG_LEVEL_WARN, LOG_COLOR_BLUE};

static KeyState *GetKeyState(InputContext *inputContext, int keyId)
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

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    for (int i = InputContexts->count - 1; i >= 0; i--)
    {
        InputContext *inputContext = (InputContext *)InputContexts->content[i];
        KeyState *keyState = GetKeyState(inputContext, key);
        if (keyState)
        {
            keyState->isPressed = action == GLFW_PRESS || action == GLFW_REPEAT;
            keyState->isDown = keyState->isPressed;
            Log(&_logConfig, "Key.isDown %d state changed: %d\n", key, keyState->isDown);
            keyState->isReleased = action == GLFW_RELEASE;
        }

        if (inputContext->blockEvents)
        {
            break;
        }
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    for (int i = InputContexts->count - 1; i >= 0; i--)
    {
        InputContext *inputContext = (InputContext *)InputContexts->content[i];
        ButtonState *buttonState = GetButtonState(inputContext, button);
        if (buttonState)
        {
            buttonState->isPressed = action == GLFW_PRESS || action == GLFW_REPEAT;
            buttonState->isDown = buttonState->isPressed;
            buttonState->isReleased = action == GLFW_RELEASE;
        }

        if (inputContext->blockEvents)
        {
            break;
        }
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    for (int i = InputContexts->count - 1; i >= 0; i--)
    {
        InputContext *inputContext = (InputContext *)InputContexts->content[i];
        for (int i = 0; i < inputContext->motionCount; i++)
        {
            inputContext->motions[i].x = xpos;
            inputContext->motions[i].y = ypos;
        }

        if (inputContext->blockEvents)
        {
            break;
        }
    }
}

void InputManager_Init(GLFWwindow *window)
{
    InputContexts = Stack_Create();
    LogInit(&_logConfig, "Input Manager Initialized.");
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
}

void InputManager_Free()
{
    Stack_Free(InputContexts);
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
    inputContext->blockEvents = false;
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

void InputManager_ResetInputs()
{
    Log(&_logConfig, "Resetting input states...");
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