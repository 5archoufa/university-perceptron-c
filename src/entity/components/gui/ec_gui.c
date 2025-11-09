#include "entity/components/gui/ec_gui.h"
// Perceptron
#include "perceptron.h"
// Entity
#include "entity/entity.h"
// World
#include "world/world.h"
// Camera (for RenderTarget)
#include "entity/components/ec_camera/ec_camera.h"
// Rendering
#include "rendering/shader/shader-manager.h"
#include "rendering/mesh/mesh-manager.h"
// GUI Widgets
#include "entity/components/gui/w_text.h"
// Math
#include <cglm/cglm.h>
// Logging
#include "logging/logger.h"

// -------------------------
// Static Variables
// -------------------------

static LogConfig _logConfig = {"EC_GUI", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Utilities
// -------------------------

static void EC_GUI_InitShader(EC_GUI *ec_gui)
{
    // Get the UI text shader (will be created in ShaderManager)
    Shader *uiShader = ShaderManager_Get(SHADER_UI_TEXT);
    if (uiShader)
    {
        ec_gui->uiShaderProgram = uiShader->shaderProgram;

        // Cache uniform locations
        ec_gui->uiModelLoc = glGetUniformLocation(ec_gui->uiShaderProgram, "uModel");
        ec_gui->uiTextureLoc = glGetUniformLocation(ec_gui->uiShaderProgram, "uTexture");
        ec_gui->uiColorLoc = glGetUniformLocation(ec_gui->uiShaderProgram, "uColor");
    }
    else
    {
        LogError(&_logConfig, "Failed to get UI text shader!");
    }
}

// -------------------------
// Anchor Utilities
// -------------------------

static V2 EC_GUI_GetAnchorPosition(EC_GUI *ec_gui, Widget_Anchor anchor)
{
    V2 canvasSize = ec_gui->resolution;

    switch (anchor)
    {
    case WIDGET_ANCHOR_TOP_LEFT:
        return (V2){0.0f, canvasSize.y};
    case WIDGET_ANCHOR_TOP_CENTER:
        return (V2){canvasSize.x * 0.5f, canvasSize.y};
    case WIDGET_ANCHOR_TOP_RIGHT:
        return (V2){canvasSize.x, canvasSize.y};
    case WIDGET_ANCHOR_MIDDLE_LEFT:
        return (V2){0.0f, canvasSize.y * 0.5f};
    case WIDGET_ANCHOR_MIDDLE_CENTER:
        return (V2){canvasSize.x * 0.5f, canvasSize.y * 0.5f};
    case WIDGET_ANCHOR_MIDDLE_RIGHT:
        return (V2){canvasSize.x, canvasSize.y * 0.5f};
    case WIDGET_ANCHOR_BOTTOM_LEFT:
        return (V2){0.0f, 0.0f};
    case WIDGET_ANCHOR_BOTTOM_CENTER:
        return (V2){canvasSize.x * 0.5f, 0.0f};
    case WIDGET_ANCHOR_BOTTOM_RIGHT:
        return (V2){canvasSize.x, 0.0f};
    default:
        return (V2){canvasSize.x * 0.5f, canvasSize.y * 0.5f}; // Default to center
    }
}

static void EC_GUI_CalculateRectTransformFromTransform(EC_GUI_Widget *widget)
{
    // Get entity's world position (assuming it's already in GUI space)
    V3 worldPos = EC_WPos(widget->component);
    V2 targetPosition = (V2){worldPos.x, worldPos.y};

    // Get anchor position in GUI space
    V2 anchorPos = EC_GUI_GetAnchorPosition(widget->ec_gui, widget->rectTransform.anchor);

    // Calculate offset from anchor to target position
    V2 offset = V2_SUB(targetPosition, anchorPos);

    // Apply pivot offset (pivot determines which part of the widget is positioned)
    V2 pivotOffset = (V2){
        widget->rectTransform.sizeDelta.x * (widget->rectTransform.pivot.x - 0.5f),
        widget->rectTransform.sizeDelta.y * (widget->rectTransform.pivot.y - 0.5f)};

    offset = V2_SUB(offset, pivotOffset);

    // Set rect transform values
    widget->rectTransform.left = offset.x;
    widget->rectTransform.right = offset.x + widget->rectTransform.sizeDelta.x;
    widget->rectTransform.bottom = offset.y;
    widget->rectTransform.top = offset.y + widget->rectTransform.sizeDelta.y;
}

static V2 EC_GUI_GetWidgetWorldPosition(EC_GUI_Widget *widget)
{
    // Get the anchor position in GUI space
    V2 anchorPos = widget->rectTransform.anchorPos;

    // Calculate position based on left/bottom (bottom-left corner of widget)
    V2 bottomLeft = (V2){
        anchorPos.x + widget->rectTransform.left,
        anchorPos.y + widget->rectTransform.bottom};

    // Apply pivot offset to get the actual widget position
    V2 pivotOffset = (V2){
        widget->rectTransform.sizeDelta.x * widget->rectTransform.pivot.x,
        widget->rectTransform.sizeDelta.y * widget->rectTransform.pivot.y};

    return (V2){bottomLeft.x + pivotOffset.x, bottomLeft.y + pivotOffset.y};
}

// -------------------------
// Creation & Freeing
// -------------------------

static void EC_GUI_Free(Component *component)
{
    EC_GUI *ec_gui = component->self;
    // Log
    LogFree(&_logConfig, "%s", component->entity->name);

    // Unregister from world
    World_GUI_Remove(ec_gui);

    free(ec_gui->widgets);
    free(ec_gui);
}

static void EC_GUI_Widget_Free(Component *component)
{
    EC_GUI_Widget *widget = component->self;
    free(widget);
}

EC_GUI_Widget *EC_GUI_Widget_Create(EC_GUI *ec_gui, Entity *entity, void *self)
{
    EC_GUI_Widget *widget = malloc(sizeof(EC_GUI_Widget));
    widget->self = self;

    // Initialize default rect transform values
    widget->rectTransform.sizeDelta = V2_ZERO;
    widget->rectTransform.pivot = V2_ZERO;
    widget->rectTransform.anchor = WIDGET_ANCHOR_MIDDLE_CENTER;
    widget->rectTransform.anchorPos = EC_GUI_GetAnchorPosition(ec_gui, widget->rectTransform.anchor);

    // Check if the self object is a W_Text widget with a component
    EC_Type widgetType = EC_T_GUI_WIDGET; // Default type
    if (self)
    {
        // For W_Text widgets, get the component type from the W_Text structure
        W_Text *w_text = (W_Text *)self;
        if (w_text->component)
        {
            widgetType = w_text->component->type;
        }
    }

    // Component
    widget->component = Component_Create(widget, entity, widgetType, EC_GUI_Widget_Free, NULL, NULL, NULL, NULL, NULL);
    // Add to GUI
    EC_GUI_AddWidget(ec_gui, widget);
    // Calculate rect transform from entity's world position
    EC_GUI_CalculateRectTransformFromTransform(widget);
    // Log
    LogCreate(&_logConfig, "%s", entity->name);
    return widget;
}

EC_GUI *EC_GUI_Create(Entity *entity, EC_GUI_RenderMode renderMode, V2 resolution)
{
    EC_GUI *ec_gui = malloc(sizeof(EC_GUI));
    ec_gui->widgets = NULL;
    ec_gui->widgets_size = 0;

    // Canvas properties
    ec_gui->renderMode = renderMode;
    ec_gui->resolution = resolution;
    ec_gui->scaleFactor = 1.0f;
    ec_gui->sortingOrder = 0;

    // Setup UI shader and cache uniform locations
    ec_gui->uiShaderProgram = 0;
    ec_gui->uiModelLoc = -1;
    ec_gui->uiTextureLoc = -1;
    ec_gui->uiColorLoc = -1;
    EC_GUI_InitShader(ec_gui);

    // Component
    ec_gui->component = Component_Create(ec_gui, entity, EC_T_GUI, EC_GUI_Free, NULL, NULL, NULL, NULL, NULL);

    // Register with world
    World_GUI_Add(ec_gui);

    // Log
    LogCreate(&_logConfig, "%s", entity->name);
    return ec_gui;
}

// -------------------------
// Canvas Functions
// -------------------------

void EC_GUI_SetRenderMode(EC_GUI *ec_gui, EC_GUI_RenderMode renderMode)
{
    ec_gui->renderMode = renderMode;
}

void EC_GUI_SetReferenceResolution(EC_GUI *ec_gui, V2 resolution)
{
    ec_gui->resolution = resolution;
}

void EC_GUI_SetSortingOrder(EC_GUI *ec_gui, int sortingOrder)
{
    ec_gui->sortingOrder = sortingOrder;
}

// -------------------------
// Rendering
// -------------------------

void EC_GUI_Render(EC_GUI *ec_gui, RenderTarget *renderTarget)
{
    if (ec_gui->widgets_size == 0)
        return;

    // Enable blending for UI
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (ec_gui->renderMode == GUI_RENDER_MODE_SCREEN_SPACE_OVERLAY)
    {
        glDisable(GL_DEPTH_TEST);
    }
    else if (ec_gui->renderMode == GUI_RENDER_MODE_WORLD_SPACE)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // Render both sides for UI elements
    }

    // Use UI shader
    glUseProgram(ec_gui->uiShaderProgram);

    // Render all widgets
    for (size_t i = 0; i < ec_gui->widgets_size; i++)
    {
        EC_GUI_Widget *widget = &ec_gui->widgets[i];
        Entity *entity = widget->component->entity;

        mat4 model;

        if (ec_gui->renderMode == GUI_RENDER_MODE_WORLD_SPACE)
        {
            // Use entity's world transform (includes position, rotation, scale)
            T_WMatrix(&entity->transform, model);
        }
        else
        {
            // Screen space overlay - create model matrix from entity position
            // Entity position is already in screen coordinates
            V3 pos = T_WPos(&entity->transform);

            glm_mat4_identity(model);
            glm_translate(model, (vec3){pos.x, pos.y, pos.z});
        }

        // Render widget based on type
        if (widget->component->type == EC_T_W_TEXT)
        {
            W_Text *w_text = (W_Text *)widget->self;
            W_Text_Render(w_text, ec_gui, model);
        }
    }

    // Restore state after GUI rendering
    if (ec_gui->renderMode == GUI_RENDER_MODE_WORLD_SPACE)
    {
        // Re-enable depth testing for any subsequent rendering
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// -------------------------
// Widgets
// -------------------------

void EC_GUI_AddWidget(EC_GUI *ec_gui, EC_GUI_Widget *widget)
{
    widget->ec_gui = ec_gui;
    ec_gui->widgets = realloc(ec_gui->widgets, sizeof(EC_GUI_Widget) * (ec_gui->widgets_size + 1));
    ec_gui->widgets[ec_gui->widgets_size] = *widget;
    ec_gui->widgets_size++;
}

void EC_GUI_RemoveWidget(EC_GUI *ec_gui, EC_GUI_Widget *widget)
{
    size_t index = -1;
    for (size_t i = 0; i < ec_gui->widgets_size; i++)
    {
        if (ec_gui->widgets[i].component == widget->component)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        LogWarning(&_logConfig, "Could not remove Widget: Widget '%s' not found in GUI", widget->component->entity->name);
        return;
    }

    widget->ec_gui = NULL;
    // Shift remaining widgets
    for (size_t i = index; i < ec_gui->widgets_size - 1; i++)
    {
        ec_gui->widgets[i] = ec_gui->widgets[i + 1];
    }
    ec_gui->widgets_size--;
    ec_gui->widgets = realloc(ec_gui->widgets, sizeof(EC_GUI_Widget) * ec_gui->widgets_size);
}

// -------------------------
// Prefabs
// -------------------------

EC_GUI *Prefab_GUI(Entity *parent, char* name, EC_GUI_RenderMode renderMode, V2 resolution, bool useGUILayer)
{
    Entity *entity = Entity_Create(parent, true, name, TS_WORLD, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    EC_GUI *ec_gui = EC_GUI_Create(entity, renderMode, resolution);
    if (useGUILayer)
    {
        Entity_SetLayer(entity, E_LAYER_GUI, true);
    }
    return ec_gui;
}

// -------------------------
// Widget Position
// -------------------------

V3 Widget_GetPosition(EC_GUI_Widget *widget)
{
    V2 worldPos = EC_GUI_GetWidgetWorldPosition(widget);
    return (V3){worldPos.x, worldPos.y, 0.0f};
}