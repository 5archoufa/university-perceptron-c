#include "entity/components/camera/camera.h"
#include "perceptron.h"
// C Libraries
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
// Entities & Components
#include "entity/entity.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/renderer/bounds.h"
#include "utilities/math/stupid_math.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
// Shaders
#include "rendering/shader/shader.h"
#include "rendering/shader/shader-manager.h"
// Materials
#include "rendering/material/material.h"
// Lighting
#include "entity/components/lighting/ls_directional.h"
#include "entity/components/lighting/ls_point.h"
// Logging
#include "logging/logger.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// -------------------------
// Static Variables
// -------------------------

const int CAMERA_RENDER_MODE_COUNT = 3;
static LogConfig _logConfig = {"EC_Camera", LOG_LEVEL_WARN, LOG_COLOR_BLUE};
const vec3 VEC_UP = {0.0f, 1.0f, 0.0f};
const vec3 VEC_RIGHT = {1.0f, 0.0f, 0.0f};
const vec3 VEC_FORWARD = {0.0f, 0.0f, -1.0f};

// -------------------------
// Utilities
// -------------------------

static inline ShaderPropSmallValue GetSmallValue(Shader *shader, Material *material, int i, int *j, bool *mustUpload)
{
    ShaderPropSmallValue value;
    if (*j < material->instanceProps_size && i == material->instanceProps[*j].shaderPropIndex) // Use instance value
    {
        (*j)++;
        value = material->instanceProps[*j - 1].smallValue;
    }
    else // Use shader default value
    {
        value = shader->properties[i].smallValue_default;
    }
    // ChatGPT, implement proper checking for every ShaderPropSmallValue type and update mustUpload accordingly
    ShaderProperty *prop = &shader->properties[i];
    switch (prop->type)
    {
    case MPT_FLOAT:
        *mustUpload = (value.floatValue != prop->smallValue_previous.floatValue);
        break;
    case MPT_VEC2:
        *mustUpload = memcmp(&value.vec2Value, &prop->smallValue_previous.vec2Value, sizeof(vec2)) != 0;
        break;
    case MPT_VEC3:
        *mustUpload = memcmp(&value.vec3Value, &prop->smallValue_previous.vec3Value, sizeof(vec3)) != 0;
        break;
    case MPT_VEC4:
        *mustUpload = memcmp(&value.vec4Value, &prop->smallValue_previous.vec4Value, sizeof(vec4)) != 0;
        break;
    case MPT_INT:
        *mustUpload = (value.intValue != prop->smallValue_previous.intValue);
        break;
    case MPT_IVEC2:
        *mustUpload = memcmp(&value.ivec2Value, &prop->smallValue_previous.ivec2Value, sizeof(ivec2)) != 0;
        break;
    case MPT_IVEC3:
        *mustUpload = memcmp(&value.ivec3Value, &prop->smallValue_previous.ivec3Value, sizeof(ivec3)) != 0;
        break;
    case MPT_IVEC4:
        *mustUpload = memcmp(&value.ivec4Value, &prop->smallValue_previous.ivec4Value, sizeof(ivec4)) != 0;
        break;
    case MPT_UINT:
        *mustUpload = (value.uintValue != prop->smallValue_previous.uintValue);
        break;
    case MPT_MAT2:
        *mustUpload = memcmp(&value.mat2Value, &prop->smallValue_previous.mat2Value, sizeof(mat2)) != 0;
        break;
    case MPT_MAT3:
        *mustUpload = memcmp(&value.mat3Value, &prop->smallValue_previous.mat3Value, sizeof(mat3)) != 0;
        break;
    case MPT_SAMPLER2D:
        *mustUpload = (value.sampler2DValue != prop->smallValue_previous.sampler2DValue);
        break;
    default:
        LogError(&_logConfig, "Unsupported ShaderPropertyType %d", prop->type);
        *mustUpload = false;
        break;
    }
    return value;
}

static inline ShaderPropBigValue *GetBigValue(Shader *shader, Material *material, int i, int *j, bool *mustUpload)
{
    ShaderPropBigValue *bigValue = NULL;
    if (*j < material->instanceProps_size && i == material->instanceProps[*j].shaderPropIndex) // Use instance value
    {
        (*j)++;
        bigValue = &material->instanceProps[*j - 1].bigValue;
    }
    else // Use shader default value
    {
        bigValue = &shader->properties[i].bigValue_default;
    }
    // Decide whether or not must upload data by comparing with previously uploaded
    *mustUpload = memcmp(bigValue->value, shader->properties[i].bigValue_previous.value, bigValue->size) != 0;
    return bigValue;
}

static void BlitToScreen(EC_Camera *ec_camera)
{
    // Disable depth test for 2D blitting
    glDisable(GL_DEPTH_TEST);

    // Calculate aspect ratios to maintain proper scaling
    float renderTargetAspect = (float)ec_camera->renderTarget->width / (float)ec_camera->renderTarget->height;
    float windowAspect = (float)WindowConfig.windowWidth / (float)WindowConfig.windowHeight;

    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = WindowConfig.windowWidth;
    int viewportHeight = WindowConfig.windowHeight;

    // Preserve aspect ratio with letterboxing (black bars)
    if (windowAspect > renderTargetAspect)
    {
        // Window is wider than render target - add pillarboxing (vertical black bars on sides)
        viewportWidth = (int)(WindowConfig.windowHeight * renderTargetAspect);
        viewportX = (WindowConfig.windowWidth - viewportWidth) / 2;
    }
    else if (windowAspect < renderTargetAspect)
    {
        // Window is taller than render target - add letterboxing (horizontal black bars on top/bottom)
        viewportHeight = (int)(WindowConfig.windowWidth / renderTargetAspect);
        viewportY = (WindowConfig.windowHeight - viewportHeight) / 2;
    }

    // Set viewport for pixel art scaling with aspect ratio preservation
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Use blit shader and cache it
    glUseProgram(ec_camera->blitShaderProgram);
    ec_camera->boundShaderProgram = ec_camera->blitShaderProgram;

    // Bind quad VAO and render target texture
    glBindVertexArray(ec_camera->quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ec_camera->renderTarget->colorTexture);
    
    // Use cached uniform location
    glUniform1i(ec_camera->blitScreenTextureLoc, 0);
    
    // Draw fullscreen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}

// -------------------------
// Rendering
// -------------------------

static void Camera_Render_OpenGL(EC_Camera *ec_camera, EC_Renderer3D *ec_renderer3d)
{
    Material *material = ec_renderer3d->material;
    Shader *shader = material->shader;
    Entity *entity = ec_renderer3d->component->entity;
    Mesh *mesh = ec_renderer3d->mesh;

    // ==== Bind the right Shader ==== //
    if (ec_camera->boundShaderProgram != shader->shaderProgram)
    {
        glUseProgram(shader->shaderProgram);
        ec_camera->boundShaderProgram = shader->shaderProgram;
    }

    // ==== Upload Shader Properties ==== //
    int j = 0; // Index of the latest instance property inside material->instanceProps
    ShaderProperty *prop;
    ShaderPropBigValue *bigValue;
    ShaderPropSmallValue smallValue;
    bool mustUpload;
    for (int i = 0; i < shader->properties_size; i++)
    {
        prop = &shader->properties[i];
        if (prop->isBig)
        {
            bigValue = GetBigValue(shader, material, i, &j, &mustUpload);
            if (!mustUpload)
            {
                continue;
            }
            shader->properties[i].bigValue_previous = *bigValue;
            // Set uniform using bigValue
            switch (prop->type)
            {
            case MPT_MAT4:
                glUniformMatrix4fv(shader->properties[i].loc, 1, GL_FALSE, (float *)bigValue);
                break;
            default:
                LogError(&_logConfig, "Error: Unsupported big property type %d for property '%s'", prop->type, prop->name);
                break;
            }
        }
        else
        {
            smallValue = GetSmallValue(shader, material, i, &j, &mustUpload);
            if (!mustUpload)
            {
                continue;
            }
            shader->properties[i].smallValue_previous = smallValue;
            switch (prop->type)
            {
            case MPT_FLOAT:
                glUniform1f(shader->properties[i].loc, smallValue.floatValue);
                break;
            case MPT_VEC2:
                glUniform2fv(shader->properties[i].loc, 1, smallValue.vec2Value);
                break;
            case MPT_VEC3:
                glUniform3fv(shader->properties[i].loc, 1, smallValue.vec3Value);
                break;
            case MPT_VEC4:
                glUniform4fv(shader->properties[i].loc, 1, smallValue.vec4Value);
                break;
            case MPT_INT:
                glUniform1i(shader->properties[i].loc, smallValue.intValue);
                break;
            case MPT_IVEC2:
                glUniform2iv(shader->properties[i].loc, 1, smallValue.ivec2Value);
                break;
            case MPT_IVEC3:
                glUniform3iv(shader->properties[i].loc, 1, smallValue.ivec3Value);
                break;
            case MPT_IVEC4:
                glUniform4iv(shader->properties[i].loc, 1, smallValue.ivec4Value);
                break;
            case MPT_UINT:
                glUniform1ui(shader->properties[i].loc, smallValue.uintValue);
                break;
            case MPT_SAMPLER2D:
                glUniform1i(shader->properties[i].loc, smallValue.sampler2DValue);
                break;
            case MPT_MAT2:
                glUniformMatrix2fv(shader->properties[i].loc, 1, GL_FALSE, (float *)smallValue.mat2Value);
                break;
            case MPT_MAT3:
                glUniformMatrix3fv(shader->properties[i].loc, 1, GL_FALSE, (float *)smallValue.mat3Value);
                break;
            default:
                LogError(&_logConfig, "Error: Unsupported small property type %d for property '%s'", prop->type, prop->name);
                break;
            }
        }
    }

    // ============ Upload Model Matrix to UBO ============ // 
    T_WMatrix(&entity->transform, shader->model);
    glUniformMatrix4fv(shader->modelLoc, 1, GL_FALSE, (float *)shader->model);

    // -------------------------
    // Draw
    // -------------------------
    // -------------------------
    // Draw
    // -------------------------
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

static void Camera_LateUpdate(Component *component)
{
    Log(&_logConfig, "Camera LateUpdate(Deltatime: %.4f)", DeltaTime);
    EC_Camera *ec_camera = component->self;
    
    glBindFramebuffer(GL_FRAMEBUFFER, ec_camera->renderTarget->FBO);
    
    glViewport(0, 0, ec_camera->renderTarget->width, ec_camera->renderTarget->height);
    // Clear with a nice sky color
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Light blue sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // ============ Prepare Global Data ============ // 
    ShaderGlobalData *globalData = ShaderManager_GetGlobalData();

    // ============ Upload Global Data :: World ============ // 
    globalData->time = PerceptronTime;

    // ============ Prepare Global Data :: Camera ============ // 
    // Camera Position
    V3 cameraPos_V3 = EC_WPos(ec_camera->component);
    vec3 cameraPos_vec3 = {cameraPos_V3.x, cameraPos_V3.y, cameraPos_V3.z};
    memcpy(globalData->camera_position, cameraPos_vec3, sizeof(vec3));
    // Camera View
    V3 f = T_Forward(&ec_camera->component->entity->transform);
    V3 r = V3_NORM(V3_CROSS(WORLD_UP, f));
    V3 u = V3_CROSS(f, r);
    // Build look-at manually
    vec3 center = {cameraPos_vec3[0] + f.x, cameraPos_vec3[1] + f.y, cameraPos_vec3[2] + f.z};
    vec3 up = {u.x, u.y, u.z};
    // Build view and projection matrices (cglm produces column-major, which is correct for OpenGL)
    glm_lookat(cameraPos_vec3, center, up, globalData->view);
    glm_perspective(ec_camera->FOV,
                    ec_camera->viewport.x / ec_camera->viewport.y,
                    ec_camera->nearClip,
                    ec_camera->farClip,
                    ec_camera->proj);
    memcpy(globalData->projection, ec_camera->proj, sizeof(mat4));

    // ============ Upload Global Data :: Directional Lights ============ // 
    int lights_count = ec_camera->world->lights_directional_size;
    EC_Light **lights = ec_camera->world->lights_directional;
    int uploadCount = 0;
    for (int i = 0; i < lights_count; i++)
    {
        if (uploadCount > LIGHT_DIRECT_COUNT_MAX)
        {
            break;
        }
        // Skip Inactive Colors
        if (!lights[i]->component->isActive)
            continue;
        EC_Light *ec_light = lights[i];
        LS_Directional *ls_directional = (LS_Directional *)ec_light->lightSource;
        // Color
        globalData->light_directional_colors[uploadCount][0] = ls_directional->color_openGL[0];
        globalData->light_directional_colors[uploadCount][1] = ls_directional->color_openGL[1];
        globalData->light_directional_colors[uploadCount][2] = ls_directional->color_openGL[2];
        // Intensity
        globalData->light_directional_colors[uploadCount][3] = ls_directional->intensity;
        // Direction - Send raw forward direction
        V3 lightDir = EC_Forward(ec_light->component);
        globalData->light_directional_directions[uploadCount][0] = lightDir.x;
        globalData->light_directional_directions[uploadCount][1] = lightDir.y;
        globalData->light_directional_directions[uploadCount][2] = lightDir.z;
        
        // Update upload count
        uploadCount++;
    }
    // Update Global Data :: Directional Light count
    globalData->light_directional_count = uploadCount;

    // ============ Update Global Data :: Point Lights ============ // 
    lights_count = ec_camera->world->lights_point_size;
    lights = ec_camera->world->lights_point;
    uploadCount = 0;
    for (int i = 0; i < lights_count; i++)
    {
        if (uploadCount > LIGHT_POINT_COUNT_MAX)
        {
            break;
        }
        // Skip Inactive Colors
        if (!lights[i]->component->isActive)
            continue;
        EC_Light *ec_light = lights[i];
        LS_Point *ls_point = (LS_Point *)ec_light->lightSource;
        // Color
        globalData->light_point_colors[uploadCount][0] = ls_point->color_openGL[0];
        globalData->light_point_colors[uploadCount][1] = ls_point->color_openGL[1];
        globalData->light_point_colors[uploadCount][2] = ls_point->color_openGL[2];
        // Intensity
        globalData->light_point_colors[uploadCount][3] = ls_point->intensity;
        // Position
        V3 position = EC_WPos(ec_light->component);
        globalData->light_point_positions[uploadCount][0] = position.x;
        globalData->light_point_positions[uploadCount][1] = position.y;
        globalData->light_point_positions[uploadCount][2] = position.z;
        // Range
        globalData->light_point_positions[uploadCount][3] = ls_point->range;
        // Update upload count
        uploadCount++;
    }
    // Update Global Data :: Point Light count
    globalData->light_point_count = uploadCount;

    // ============ Upload Global Data ============ // 
    ShaderManager_UploadGlobalData();

    // -------------------------
    // Render 3D Models
    // -------------------------
    int rendererCount_3D = ec_camera->world->rendererCount_3D;
    EC_Renderer3D **renderers = ec_camera->world->renderers_3D;
    
    for (int i = 0; i < rendererCount_3D; i++)
    {
        Camera_Render_OpenGL(ec_camera, renderers[i]);
    }
    
    // Unbind VAO and textures from 3D rendering to ensure clean state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Unbind the render target framebuffer before blitting
    // This ensures we're done writing to the render target's textures
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Blit to screen
    // BlitToScreen will handle viewport scaling and aspect ratio preservation
    BlitToScreen(ec_camera);
}

V2_INT Camera_WorldToScreen_V2(EC_Camera *EC_camera, V2 *position)
{
    float x = position->x - EC_camera->position->x + EC_camera->viewport.x * 0.5;
    float y = position->y - EC_camera->position->y + EC_camera->viewport.y * 0.5;
    return (V2_INT){(int)(x * EC_camera->pixelsPerUnit), (int)(y * EC_camera->pixelsPerUnit)};
}

V2_INT Camera_WorldToScreen_V3(EC_Camera *EC_camera, V3 *position)
{
    float x = position->x - EC_camera->position->x + EC_camera->viewport.x * 0.5;
    float y = position->y - EC_camera->position->y + EC_camera->viewport.y * 0.5;
    return (V2_INT){(int)(x * EC_camera->pixelsPerUnit), (int)(y * EC_camera->pixelsPerUnit)};
}

RenderTarget *RenderTarget_Create(int width, int height)
{
    RenderTarget *rt = malloc(sizeof(RenderTarget));
    rt->width = width;
    rt->height = height;

    // Create framebuffer
    glGenFramebuffers(1, &rt->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, rt->FBO);

    // Create color texture
    glGenTextures(1, &rt->colorTexture);
    glBindTexture(GL_TEXTURE_2D, rt->colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // CRITICAL: Use GL_NEAREST for pixel-perfect scaling!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->colorTexture, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &rt->depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, rt->depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->depthRBO);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete!\n");
        free(rt);
        return NULL;
    }

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return rt;
}

void RenderTarget_Free(RenderTarget *rt)
{
    glDeleteFramebuffers(1, &rt->FBO);
    glDeleteTextures(1, &rt->colorTexture);
    glDeleteRenderbuffers(1, &rt->depthRBO);
    free(rt);
}

const char *SCREEN_TEXTURE = "screenTexture";

void SetupBlitQuad(EC_Camera *ec_camera)
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &ec_camera->quadVAO);
    glGenBuffers(1, &ec_camera->quadVBO);
    glBindVertexArray(ec_camera->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ec_camera->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    // TexCoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);
}

EC_Camera *EC_Camera_Create(Entity *entity, V2 viewport, float fov,
                            float nearClip, float farClip)
{
    EC_Camera *ec_camera = malloc(sizeof(EC_Camera));
    ec_camera->viewport = (V2){viewport.x, viewport.y};
    ec_camera->world = World_Get(entity);
    ec_camera->position = &entity->transform.w_pos;
    ec_camera->pixelsPerUnit = 50;
    ec_camera->FOV = fov;
    ec_camera->nearClip = nearClip;
    ec_camera->farClip = farClip;
    ec_camera->renderMode = CAMERA_RENDER_SOLID_WIREFRAME;
    ec_camera->areRulersVisible = false;

    // Setup render target
    ec_camera->renderTarget = RenderTarget_Create(viewport.x, viewport.y);
    SetupBlitQuad(ec_camera);

    // Load and compile blit shader
    ec_camera->blitShaderProgram = ShaderManager_Get(SHADER_SCREEN_BLIT)->shaderProgram;
    
    // Cache uniform locations
    ec_camera->blitScreenTextureLoc = glGetUniformLocation(ec_camera->blitShaderProgram, "screenTexture");

    ec_camera->component = Component_Create(ec_camera, entity, EC_T_CAMERA,
                                            EC_Camera_Free, NULL, NULL, NULL,
                                            Camera_LateUpdate, NULL);
    return ec_camera;
}

void EC_Camera_Free(Component *component)
{
    EC_Camera *camera = (EC_Camera *)component->self;
    free(camera);
}