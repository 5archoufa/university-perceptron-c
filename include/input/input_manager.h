#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

// C
#include <stdlib.h>
#include <stdbool.h>
// Utilities
#include "utilities/stack.h"
// Math
#include "utilities/math/v2.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// Input
#include "input/input_manager.h"

// -------------------------
// Types
// -------------------------

typedef struct
{
    uint32_t id;
    bool isPressed;
    bool isReleased;
    bool isDown;
} KeyState;

typedef struct
{
    uint32_t id;
    bool isPressed;
    bool isReleased;
    bool isDown;
} ButtonState;

typedef struct
{
    uint32_t id;
    V2 position;
    V2 delta;
} MotionState;

typedef struct InputMapping
{
    uint32_t contextId;
    // Keys
    size_t keys_size;
    KeyState **keys;  // Array of pointers to prevent invalidation on realloc
    // Buttons
    size_t buttons_size;
    ButtonState **buttons;  // Array of pointers to prevent invalidation on realloc
    // Motions
    size_t motions_size;
    MotionState **motions;  // Array of pointers to prevent invalidation on realloc
} InputMapping;

typedef struct InputListener
{
    char *name;
    // Mappings
    size_t mappings_size;
    InputMapping *mappings;
} InputListener;

typedef struct InputContext
{
    uint32_t id;
    char *name;
    size_t listeners_size;
    InputListener *listeners;
    /// @brief Prevents input events from going further in the stack
    bool blockEvents;
    /// @brief If true, the context will process inputs this frame
    bool isActive;
} InputContext;

typedef struct InputManager
{
    char *name;
    size_t contexts_size;
    InputContext **contexts;  // Array of pointers instead of array of structs
    Stack *contextStack;
    size_t nextContextId;
} InputManager;

// ------------------------- 
// Manager Management 
// -------------------------

void InputManager_Select(InputManager* manager);

// -------------------------
// Creation & Freeing
// -------------------------

InputManager *InputManager_Create(GLFWwindow *window, char *name);
void InputManager_Free(InputManager *manager);

InputContext *InputContext_Create(char *name, bool blockEvents);
void InputContext_Free(InputContext *inputContext);

void InputMapping_Free(InputMapping *mapping);

void InputListener_Free(InputListener *listener);

// ----------------------------------------
// Input Mapping 
// ----------------------------------------

KeyState *InputMapping_AddKey(InputMapping *mapping, uint32_t keyId);
ButtonState *InputMapping_AddButton(InputMapping *mapping, uint32_t buttonId);
MotionState *InputMapping_AddMotion(InputMapping *mapping, uint32_t motionId);

// ----------------------------------------
// Input Manager
// ----------------------------------------

void InputManager_PushContext(InputContext *context);
void InputManager_PopContext(InputContext *context);
void InputManager_ResetInputs();


// ----------------------------------------
// Input Listener 
// ----------------------------------------

InputListener *InputContext_AddListener(InputContext *context, char *name);
InputMapping *InputListener_AddMapping(InputListener *listener, uint32_t contextId);

#endif