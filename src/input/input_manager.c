#include "input/input_manager.h"
// C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// Stack
#include "utilities/stack.h"
// Logging
#include "logging/logger.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// ----------------------------------------
// Static Variables
// ----------------------------------------

static LogConfig _logConfig = {"InputManager", LOG_LEVEL_WARN, LOG_COLOR_BLUE};
static InputManager *_manager = NULL;

// -------------------------
// Manager Management
// -------------------------

void InputManager_Select(InputManager *manager)
{
    _manager = manager;
    LogSuccess(&_logConfig, "Input Manager '%s' Selected.", manager->name);
}

// -------------------------
// Input State Getters
// -------------------------

static KeyState *Mapping_GetKeyState(InputMapping *mapping, int keyId)
{
    for (size_t i = 0; i < mapping->keys_size; i++)
    {
        if (mapping->keys[i]->id == (uint32_t)keyId)
        {
            return mapping->keys[i];
        }
    }
    return NULL;
}

static ButtonState *Mapping_GetButtonState(InputMapping *mapping, int buttonId)
{
    for (size_t i = 0; i < mapping->buttons_size; i++)
    {
        if (mapping->buttons[i]->id == (uint32_t)buttonId)
        {
            return mapping->buttons[i];
        }
    }
    return NULL;
}

static KeyState *Context_GetKeyState(InputContext *context, int keyId)
{
    for (size_t i = 0; i < context->listeners_size; i++)
    {
        InputListener *listener = &context->listeners[i];
        for (size_t j = 0; j < listener->mappings_size; j++)
        {
            KeyState *keyState = Mapping_GetKeyState(&listener->mappings[j], keyId);
            if (keyState) return keyState;
        }
    }
    return NULL;
}

static ButtonState *Context_GetButtonState(InputContext *context, int buttonId)
{
    for (size_t i = 0; i < context->listeners_size; i++)
    {
        InputListener *listener = &context->listeners[i];
        for (size_t j = 0; j < listener->mappings_size; j++)
        {
            ButtonState *buttonState = Mapping_GetButtonState(&listener->mappings[j], buttonId);
            if (buttonState) return buttonState;
        }
    }
    return NULL;
}

// -------------------------
// GLFW Callbacks
// -------------------------

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (!_manager) return;
    
    for (int i = _manager->contextStack->count - 1; i >= 0; i--)
    {
        InputContext *context = (InputContext *)_manager->contextStack->content[i];
        KeyState *keyState = Context_GetKeyState(context, key);
        if (keyState)
        {
            keyState->isPressed = action == GLFW_PRESS || action == GLFW_REPEAT;
            keyState->isDown = keyState->isPressed || (keyState->isDown && action != GLFW_RELEASE);
            Log(&_logConfig, "Key.isDown %d state changed: %d\n", key, keyState->isDown);
            keyState->isReleased = action == GLFW_RELEASE;
        }

        if (context->blockEvents) break;
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!_manager) return;
    
    for (int i = _manager->contextStack->count - 1; i >= 0; i--)
    {
        InputContext *context = (InputContext *)_manager->contextStack->content[i];
        ButtonState *buttonState = Context_GetButtonState(context, button);
        if (buttonState)
        {
            buttonState->isPressed = action == GLFW_PRESS || action == GLFW_REPEAT;
            buttonState->isDown = buttonState->isPressed || (buttonState->isDown && action != GLFW_RELEASE);
            buttonState->isReleased = action == GLFW_RELEASE;
        }

        if (context->blockEvents) break;
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (!_manager) return;
    
    V2 newPos = {xpos, ypos};
    for (int i = _manager->contextStack->count - 1; i >= 0; i--)
    {
        InputContext *context = (InputContext *)_manager->contextStack->content[i];
        for (size_t l = 0; l < context->listeners_size; l++)
        {
            InputListener *listener = &context->listeners[l];
            for (size_t m = 0; m < listener->mappings_size; m++)
            {
                InputMapping *mapping = &listener->mappings[m];
                for (size_t i = 0; i < mapping->motions_size; i++)
                {
                    MotionState *motion = mapping->motions[i];
                    motion->delta = V2_SUB(newPos, motion->position);
                    motion->position = newPos;
                }
            }
        }

        if (context->blockEvents) break;
    }
}

// -------------------------
// Creation & Freeing
// -------------------------

InputManager *InputManager_Create(GLFWwindow *window,char *name)
{
    InputManager *manager = malloc(sizeof(InputManager));
    manager->name = strdup(name);
    manager->contexts_size = 0;
    manager->contexts = NULL;
    manager->contextStack = Stack_Create();
    manager->nextContextId = 0;
    // Subscrive to GLFW input events
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    // Log
    LogInit(&_logConfig, "Input Manager '%s' Created.", name);
    // Select InputManager
    InputManager_Select(manager);
    return manager;
}

void InputManager_Free(InputManager *manager)
{
    if (manager)
    {
        if (manager == _manager)
        {
            LogWarning(&_logConfig, "Freeing currently selected Input Manager '%s', unselecting...", manager->name);
            _manager = NULL;
        }
        
        // Clear the context stack first to avoid referencing freed memory by Stack_Free
        if (manager->contextStack)
        {
            manager->contextStack->count = 0;
        }
        
        // Now free all contexts
        if (manager->contexts && manager->contexts_size > 0)
        {
            for (size_t i = 0; i < manager->contexts_size; i++)
            {
                InputContext_Free(manager->contexts[i]);
            }
        }
        free(manager->contexts);
        Stack_Free(manager->contextStack);
        free(manager->name);
        LogFree(&_logConfig, "Input Manager %s Freed.", manager->name);
        free(manager);
    }
}

// -------------------------
// Context Creation & Management
// -------------------------

InputContext *InputContext_Create(char *name, bool blockEvents)
{
    if (!_manager) {
        printf("Error: InputContext_Create called but no InputManager is selected!\n");
        return NULL;
    }
    
    InputContext *context = malloc(sizeof(InputContext));
    context->id = _manager->nextContextId++;
    context->name = strdup(name);
    context->blockEvents = blockEvents;
    context->isActive = false;
    context->listeners_size = 0;
    context->listeners = NULL;
    
    // Register context with manager for automatic cleanup
    _manager->contexts_size++;
    _manager->contexts = realloc(_manager->contexts, sizeof(InputContext*) * _manager->contexts_size);
    _manager->contexts[_manager->contexts_size - 1] = context;
    
    return context;
}

void InputContext_Free(InputContext *context)
{
    if (!context) return;
        for (size_t i = 0; i < context->listeners_size; i++)
    {
        InputListener_Free(&context->listeners[i]);
    }
    free(context->listeners);
    free(context->name);
    free(context);
}

InputListener *InputContext_AddListener(InputContext *context, char *name)
{
    context->listeners_size++;
    context->listeners = realloc(context->listeners, sizeof(InputListener) * context->listeners_size);
    InputListener *listener = &context->listeners[context->listeners_size - 1];
    listener->name = strdup(name);
    listener->mappings_size = 0;
    listener->mappings = NULL;
    return listener;
}

// -------------------------
// Listener Management
// -------------------------

void InputListener_Free(InputListener *listener)
{
    if (!listener) return;
    
    for (size_t i = 0; i < listener->mappings_size; i++)
    {
        InputMapping_Free(&listener->mappings[i]);
    }
    free(listener->mappings);
    free(listener->name);
}

InputMapping *InputListener_AddMapping(InputListener *listener, uint32_t contextId)
{
    listener->mappings_size++;
    listener->mappings = realloc(listener->mappings, sizeof(InputMapping) * listener->mappings_size);
    InputMapping *mapping = &listener->mappings[listener->mappings_size - 1];
    mapping->contextId = contextId;
    mapping->keys_size = 0;
    mapping->keys = NULL;
    mapping->buttons_size = 0;
    mapping->buttons = NULL;
    mapping->motions_size = 0;
    mapping->motions = NULL;
    return mapping;
}

// -------------------------
// Mapping Management
// -------------------------

void InputMapping_Free(InputMapping *mapping)
{
    if (!mapping) return;
    
    // Free individual key states
    for (size_t i = 0; i < mapping->keys_size; i++)
    {
        free(mapping->keys[i]);
    }
    free(mapping->keys);
    
    // Free individual button states
    for (size_t i = 0; i < mapping->buttons_size; i++)
    {
        free(mapping->buttons[i]);
    }
    free(mapping->buttons);
    
    // Free individual motion states
    for (size_t i = 0; i < mapping->motions_size; i++)
    {
        free(mapping->motions[i]);
    }
    free(mapping->motions);
}

KeyState *InputMapping_AddKey(InputMapping *mapping, uint32_t keyId)
{
    mapping->keys_size++;
    mapping->keys = realloc(mapping->keys, sizeof(KeyState*) * mapping->keys_size);
    KeyState *key = malloc(sizeof(KeyState));
    key->id = keyId;
    key->isPressed = false;
    key->isReleased = false;
    key->isDown = false;
    mapping->keys[mapping->keys_size - 1] = key;
    return key;
}

ButtonState *InputMapping_AddButton(InputMapping *mapping, uint32_t buttonId)
{
    mapping->buttons_size++;
    mapping->buttons = realloc(mapping->buttons, sizeof(ButtonState*) * mapping->buttons_size);
    ButtonState *button = malloc(sizeof(ButtonState));
    button->id = buttonId;
    button->isPressed = false;
    button->isReleased = false;
    button->isDown = false;
    mapping->buttons[mapping->buttons_size - 1] = button;
    return button;
}

MotionState *InputMapping_AddMotion(InputMapping *mapping, uint32_t motionId)
{
    mapping->motions_size++;
    mapping->motions = realloc(mapping->motions, sizeof(MotionState*) * mapping->motions_size);
    MotionState *motion = malloc(sizeof(MotionState));
    motion->id = motionId;
    motion->position = V2_ZERO;
    motion->delta = V2_ZERO;
    mapping->motions[mapping->motions_size - 1] = motion;
    return motion;
}

// -------------------------
// State Management
// -------------------------

static void UpdateActiveStates()
{
    if (!_manager) return;
    
    LogWarning(&_logConfig, "Updating %d Input Contexts active states...", _manager->contextStack->count);
    for (int i = _manager->contextStack->count - 1; i >= 0; i--)
    {
        InputContext *context = (InputContext *)_manager->contextStack->content[i];
        context->isActive = true;
        LogWarning(&_logConfig, "Input Context '%s' isActive set to true", context->name);
        if (context->blockEvents)
        {
            for (int j = i - 1; j >= 0; j--)
            {
                context = (InputContext *)_manager->contextStack->content[j];
                context->isActive = false;
                LogWarning(&_logConfig, "Input Context '%s' isActive set to false", context->name);
            }
            break;
        }
    }
}

void InputManager_PopContext(InputContext *context)
{
    if (!_manager) return;
    
    LogSuccess(&_logConfig, "PopContext(%s)\n", context->name);
    _manager->contextStack->Pop(_manager->contextStack, context);
    UpdateActiveStates();
}

void InputManager_PushContext(InputContext *context)
{
    if (!_manager) return;
    
    LogSuccess(&_logConfig, "PushContext(%s)\n", context->name);
    _manager->contextStack->Push(_manager->contextStack, (void *)context);
    UpdateActiveStates();
}

void InputManager_ResetInputs()
{
    if (!_manager) return;
    
    Log(&_logConfig, "Resetting input states...");
    for (int i = _manager->contextStack->count - 1; i >= 0; i--)
    {
        InputContext *context = (InputContext *)_manager->contextStack->content[i];
        for (size_t l = 0; l < context->listeners_size; l++)
        {
            InputListener *listener = &context->listeners[l];
            for (size_t m = 0; m < listener->mappings_size; m++)
            {
                InputMapping *mapping = &listener->mappings[m];
                for (size_t k = 0; k < mapping->keys_size; k++)
                {
                    mapping->keys[k]->isPressed = false;
                    mapping->keys[k]->isReleased = false;
                }
                for (size_t b = 0; b < mapping->buttons_size; b++)
                {
                    mapping->buttons[b]->isPressed = false;
                    mapping->buttons[b]->isReleased = false;
                }
                for (size_t mo = 0; mo < mapping->motions_size; mo++)
                {
                    mapping->motions[mo]->delta = V2_ZERO;
                }
            }
        }
    }
}