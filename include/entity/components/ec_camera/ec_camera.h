#ifndef EC_CAMERA_H
#define EC_CAMERA_H
// Math
#include "utilities/math/v3.h"
#include "utilities/math/v2.h"
// Color
#include "rendering/color.h"
// C
#include <stdbool.h>
// Window
#include "ui/window.h"
// Entity
#include "entity/entity.h"
// AABB
#include "physics/aabb.h"
// World
#include "world/world.h"
// Render Target
#include "rendering/render_target.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// ------------------------- 
// Constants 
// -------------------------

extern const int CAMERA_RENDER_MODE_COUNT;

// ------------------------- 
// Types
// -------------------------

typedef struct EC_Camera EC_Camera;
struct EC_Camera
{
    Component *component;
    V2 viewport;
    World *world;
    float pixelsPerUnit;
    V3 *position;
    // Render Modes
    bool renderSolid;
    bool renderWireframe;
    bool renderSkybox;
    RGBA backgroundColor;
#ifdef DEBUG_COLLIDERS
    bool renderColliders;
#endif
    // Render Target
    RenderTarget *renderTarget;
    // Clipping Planes
    float nearClip;
    float farClip;
    /// @brief Field of View in Radians
    float FOV;
    // Culling Mask
    uint32_t cullingMask;
    // Blitting
    GLuint blitShaderProgram;
    GLuint blitTextureLoc;
    GLuint quadVAO;
    GLuint quadVBO;
    /// @brief If true, will write GUI elements to the screen directly instead of the renderTarget for higher resolution
    bool writeGUIToScreen;
    // Cache
    GLuint boundShaderProgram;
    mat4 proj;
    V3 backward;
    V3 up;
    V3 right;
};

// ------------------------- 
// Creation & Freeing 
// -------------------------

EC_Camera *Prefab_RenderTargetCamera(Entity *parent, char *e_name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V2 viewport, RenderTarget *renderTarget, Shader *blitShader, RGBA backgroundColor);
EC_Camera *Prefab_DisplayCamera(Entity *parent, char *e_name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V2 viewport);

// ------------------------- 
// Culling Mask 
// -------------------------

void EC_Camera_AddToCullingMask(EC_Camera *ec_camera, uint8_t layer);
void EC_Camera_RemoveFromCullingMask(EC_Camera *ec_camera, uint8_t layer);
void EC_Camera_ClearCullingMask(EC_Camera *ec_camera);

#endif