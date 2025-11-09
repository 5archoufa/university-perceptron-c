#ifndef UI_GUI_H
#define UI_GUI_H

// Entity
#include "entity/entity.h"
// Math
#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
// Rendering
#include "rendering/render_target.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// -------------------------
// Forward Declarations
// -------------------------

typedef struct EC_GUI EC_GUI;

// -------------------------
// Types
// -------------------------

typedef enum
{
    WIDGET_ANCHOR_TOP_LEFT,
    WIDGET_ANCHOR_TOP_CENTER,
    WIDGET_ANCHOR_TOP_RIGHT,
    WIDGET_ANCHOR_MIDDLE_LEFT,
    WIDGET_ANCHOR_MIDDLE_CENTER,
    WIDGET_ANCHOR_MIDDLE_RIGHT,
    WIDGET_ANCHOR_BOTTOM_LEFT,
    WIDGET_ANCHOR_BOTTOM_CENTER,
    WIDGET_ANCHOR_BOTTOM_RIGHT
} Widget_Anchor;

typedef struct
{
    // SizeDelta
    V2 sizeDelta;
    // Pivot
    V2 pivot;
    // Anchor
    Widget_Anchor anchor;
    /// @brief Anchor Position in GUI space
    V2 anchorPos;
    // Position in GUI space relative to the anchor
    float top;
    float bottom;
    float right;
    float left;
} RectTransform;

typedef struct
{
    Component *component;
    EC_GUI *ec_gui;
    // Self
    void *self;
    RectTransform rectTransform;
} EC_GUI_Widget;

typedef enum
{
    /// @brief Always renders on top, ignores camera
    GUI_RENDER_MODE_SCREEN_SPACE_OVERLAY,
    /// @brief Renders in 3D world space
    GUI_RENDER_MODE_WORLD_SPACE
} EC_GUI_RenderMode;

struct EC_GUI
{
    Component *component;
    EC_GUI_Widget *widgets;
    size_t widgets_size;

    // Canvas properties
    EC_GUI_RenderMode renderMode;
    V2 resolution;
    float scaleFactor;
    int sortingOrder;

    // Rendering
    GLuint uiShaderProgram;

    // Cached uniform locations for UI shader
    GLint uiModelLoc;
    GLint uiTextureLoc;
    GLint uiColorLoc;
};

// -------------------------
// Creation & Freeing
// -------------------------

EC_GUI_Widget *EC_GUI_Widget_Create(EC_GUI *ec_gui, Entity *entity, void *self);
EC_GUI *EC_GUI_Create(Entity *entity, EC_GUI_RenderMode renderMode, V2 resolution);

// -------------------------
// Canvas Functions
// -------------------------

void EC_GUI_SetRenderMode(EC_GUI *ec_gui, EC_GUI_RenderMode renderMode);
void EC_GUI_SetReferenceResolution(EC_GUI *ec_gui, V2 resolution);
void EC_GUI_SetSortingOrder(EC_GUI *ec_gui, int sortingOrder);
void EC_GUI_UpdateProjectionMatrix(EC_GUI *ec_gui, V2 screenSize);

// -------------------------
// Setters
// -------------------------

V3 Widget_GetPosition(EC_GUI_Widget *widget);

// -------------------------
// Rendering
// -------------------------

void EC_GUI_Render(EC_GUI *ec_gui, RenderTarget *renderTarget);

// -------------------------
// Widgets
// -------------------------

void EC_GUI_AddWidget(EC_GUI *ec_gui, EC_GUI_Widget *widget);
void EC_GUI_RemoveWidget(EC_GUI *ec_gui, EC_GUI_Widget *widget);

// -------------------------
// Prefabs
// -------------------------

EC_GUI *Prefab_GUI(Entity *parent, char* name, EC_GUI_RenderMode renderMode, V2 resolution, bool useGUILayer);

#endif