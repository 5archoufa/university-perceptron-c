#ifndef EC_CAMERA_H
#define EC_CAMERA_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"
#include <stdbool.h>
#include "ui/window.h"
#include "entity/entity.h"
#include "entity/components/renderer/bounds.h"
#include "world/world.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

typedef struct EC_Camera EC_Camera;

extern const int CAMERA_RENDER_MODE_COUNT;
typedef enum CameraRenderMode
{
    CAMERA_RENDER_WIREFRAME,
    CAMERA_RENDER_SOLID,
    CAMERA_RENDER_SOLID_WIREFRAME
} CameraRenderMode;

typedef struct
{
    GLuint FBO;
    GLuint colorTexture;
    GLuint depthRBO;
    int width;
    int height;
} RenderTarget;

struct EC_Camera
{
    Component *component;
    V2 viewport;
    World *world;
    float pixelsPerUnit;
    V3 *position;
    CameraRenderMode renderMode;
    bool areRulersVisible;
    RenderTarget *renderTarget;
    float nearClip;
    float farClip;
    /// @brief Field of View in Radians
    float FOV;
    // Blitting
    GLuint boundShaderProgram;
    GLuint blitShaderProgram;
    GLuint quadVAO;
    GLuint quadVBO;
    // Cache
    mat4 proj;
    V3 backward;
    V3 up;
    V3 right;
};

EC_Camera *EC_Camera_Create(Entity *entity, V2 viewport, float fov,
                            float nearClip, float farClip);
void EC_Camera_Free(Component *component);
V2_INT Camera_WorldToScreen_V2(EC_Camera *EC_camera, V2 *position);
V2_INT Camera_WorldToScreen_V3(EC_Camera *EC_camera, V3 *position);
void Camera_LookAt(EC_Camera *camera, V3 target);
#endif