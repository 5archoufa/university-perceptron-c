#include "entity/components/ec_camera/ec_camera.h"
#include "perceptron.h"
// C Libraries
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
// Entities & Components
#include "entity/entity.h"
#include "physics/aabb.h"
#include "utilities/math/stupid_math.h"
#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
// Shaders
#include "rendering/shader/shader.h"
#include "rendering/shader/shader-manager.h"
// Color
#include "rendering/color.h"
// Materials
#include "rendering/material/material.h"
// Physics
#include "physics/physics-manager.h"
#include "entity/components/ec_collider/ec_collider.h"
#include "entity/components/ec_rigidbody/ec_rigidbody.h"
// Lighting
#include "entity/components/lighting/ls_directional.h"
#include "entity/components/lighting/ls_point.h"
// GUI
#include "entity/components/gui/ec_gui.h"
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

inline static void UseShaderProgram(EC_Camera *ec_camera, GLuint shaderProgram)
{
    if (ec_camera->boundShaderProgram == shaderProgram)
        return;
    glUseProgram(shaderProgram);
    ec_camera->boundShaderProgram = shaderProgram;
}

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

static void BlitToQuad(EC_Camera *ec_camera)
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

    UseShaderProgram(ec_camera, ec_camera->blitShaderProgram);

    // Bind quad VAO and render target texture
    glBindVertexArray(ec_camera->quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ec_camera->renderTarget->colorTexture);

    // Use cached uniform location
    glUniform1i(ec_camera->blitTextureLoc, 0);

    // Draw fullscreen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);

    // IRestore full window viewport for GUI rendering
    glViewport(0, 0, WindowConfig.windowWidth, WindowConfig.windowHeight);
}

#ifdef DEBUG_COLLIDERS
// -------------------------
// Debug Collider Rendering
// -------------------------

static void Camera_RenderDebugColliders(EC_Camera *ec_camera)
{
    // Get physics manager and colliders
    PhysicsManager *physicsManager = ec_camera->world->physicsManager;
    if (!physicsManager)
        return;

    // Get the debug collider shader
    Shader *shader = ShaderManager_Get(SHADER_DEBUG_COLLIDER);
    if (!shader)
        return;

    // Bind shader
    UseShaderProgram(ec_camera, shader->shaderProgram);

    // Enable blending for semi-transparent faces
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable depth testing but disable depth writing for proper transparency
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // Don't write to depth buffer for transparency

    // Disable face culling to see colliders from inside
    glDisable(GL_CULL_FACE);

    // Render all colliders
    for (int i = 0; i < physicsManager->rigidbodies_size; i++)
    {
        EC_RigidBody *rigidbody = physicsManager->rigidbodies[i];
        if (!rigidbody || !rigidbody->ec_collider)
            continue;

        EC_Collider *collider = rigidbody->ec_collider;
        if (!collider->debugMesh)
            continue;

        // Get entity transform and apply collider offset
        Entity *entity = collider->component->entity;
        
        // Start with the entity's world matrix
        T_WMatrix(&entity->transform, shader->model);
        
        // Create translation matrix for the offset
        mat4 offsetMatrix;
        vec3 offset = {collider->offset.x, collider->offset.y, collider->offset.z};
        glm_translate_make(offsetMatrix, offset);
        
        // Combine: model = entity_transform * offset
        mat4 finalModel;
        glm_mat4_mul(shader->model, offsetMatrix, finalModel);
        
        // Upload the final model matrix
        glUniformMatrix4fv(shader->modelLoc, 1, GL_FALSE, (float *)finalModel);

        // Render the debug mesh as triangles
        Mesh *debugMesh = collider->debugMesh;
        glBindVertexArray(debugMesh->VAO);
        glDrawElements(GL_TRIANGLES, debugMesh->indices_size, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Restore normal rendering state
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
#endif

// -------------------------
// Rendering
// -------------------------

inline static void Render_MeshRenderer(EC_Camera *ec_camera, EC_MeshRenderer *ec_meshRenderer)
{
    // Check the layer
    if (!(ec_camera->cullingMask & (1u << ec_meshRenderer->component->entity->layer)))
        return;
    Material *material = ec_meshRenderer->material;
    Shader *shader = material->shader;
    Entity *entity = ec_meshRenderer->component->entity;
    Mesh *mesh = ec_meshRenderer->mesh;

    // ==== Bind the right Shader ==== //
    UseShaderProgram(ec_camera, shader->shaderProgram);

    // ==== BIND TEXTURES FIRST ==== //
    // We need to bind textures to texture units and set the sampler uniforms
    // The material stores texture IDs in sampler2DValue, but we need to:
    // 1. Bind each texture ID to a texture unit (GL_TEXTURE0, GL_TEXTURE1, etc.)
    // 2. Set the sampler uniform to the unit number (0, 1, 2, etc.)

    int j_texture = 0;   // Track position in material instance props
    int textureUnit = 0; // Current texture unit to bind to

    for (int i = 0; i < shader->properties_size; i++)
    {
        ShaderProperty *prop = &shader->properties[i];

        if (prop->type == MPT_SAMPLER2D)
        {
            GLuint textureID = 0;

            // Check if material has an instance value for this sampler
            if (j_texture < material->instanceProps_size &&
                i == material->instanceProps[j_texture].shaderPropIndex)
            {
                // Get the texture ID from the instance property
                textureID = material->instanceProps[j_texture].smallValue.sampler2DValue;
                j_texture++;
            }
            else
            {
                // Use shader's default texture ID
                textureID = prop->smallValue_default.sampler2DValue;
            }

            // Only bind if we have a valid texture ID
            if (textureID != 0)
            {
                // Activate the texture unit
                glActiveTexture(GL_TEXTURE0 + textureUnit);

                // Bind the texture to this unit
                glBindTexture(GL_TEXTURE_2D, textureID);

                // Set the sampler uniform to this texture unit number
                glUniform1i(prop->loc, textureUnit);

                // Move to next texture unit for next texture
                textureUnit++;
            }
        }
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

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->indices_size, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

inline static void Render_GUIs(EC_Camera *ec_camera)
{
    // ============ Render GUIs ============ //
    int guiCount = ec_camera->world->guis_size;
    EC_GUI **guis = ec_camera->world->guis;
    ShaderGlobalData *globalData = ShaderManager_GetGlobalData();

    // ============ World-Space GUIs ============ //
    for (int i = 0; i < guiCount; i++)
    {
        if (guis[i] == NULL)
            continue;
        EC_GUI *gui = guis[i];
        if (gui->renderMode != GUI_RENDER_MODE_WORLD_SPACE)
            continue;
        // Check if the gui's layer belongs to the camera's culling mask
        if (!(ec_camera->cullingMask & (1u << gui->component->entity->layer)))
        {
            continue;
        }

        // Render the GUI in world space
        EC_GUI_Render(gui, ec_camera->renderTarget);
    }

    // ============ Screen-Space GUIs ============ //
    // Save original UBO state
    mat4 savedView, savedProjection;
    glm_mat4_copy(globalData->view, savedView);
    glm_mat4_copy(globalData->projection, savedProjection);

    // Update UBO with screen-space matrices
    V2 screenSize = {(float)ec_camera->viewport.x, (float)ec_camera->viewport.y};
    // Create orthographic projection for screen space
    mat4 orthoProjection;
    glm_ortho(0.0f, screenSize.x, 0.0f, screenSize.y, -1.0f, 1.0f, orthoProjection);
    // Identity view for screen space
    mat4 identityView;
    glm_mat4_identity(identityView);
    // Temporarily update UBO
    glm_mat4_copy(identityView, globalData->view);
    glm_mat4_copy(orthoProjection, globalData->projection);
    ShaderManager_UploadGlobalData();
    // Render screen-space GUIs
    for (int i = 0; i < guiCount; i++)
    {
        if (guis[i] == NULL)
            continue;
        EC_GUI *gui = guis[i];
        if (gui->renderMode != GUI_RENDER_MODE_SCREEN_SPACE_OVERLAY)
            continue;
        // Check if the gui's layer belongs to the camera's culling mask
        if (!(ec_camera->cullingMask & (1u << gui->component->entity->layer)))
        {
            continue;
        }

        // Render the GUI as screen overlay
        EC_GUI_Render(gui, ec_camera->renderTarget);
    }

    // Restore original UBO state
    glm_mat4_copy(savedView, globalData->view);
    glm_mat4_copy(savedProjection, globalData->projection);
    ShaderManager_UploadGlobalData();

    // Unbind VAO and textures from GUI rendering to ensure clean state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void Render_MeshRenderers(EC_Camera *ec_camera)
{
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    if (ec_camera->renderWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Solid mode
    int meshRenderers_size = ec_camera->world->meshRenderers_size;
    EC_MeshRenderer **renderers = ec_camera->world->meshRenderers;
    ec_camera->boundShaderProgram = 0; // Reset bound shader program to force rebind
    for (int i = 0; i < meshRenderers_size; i++)
    {
        Render_MeshRenderer(ec_camera, renderers[i]);
    }
    // Unbind VAO and textures from 3D rendering to ensure clean state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void CalculateAndUploadGlobalData(EC_Camera *ec_camera)
{
    Transform *transform = &ec_camera->component->entity->transform;
    ShaderGlobalData *globalData = ShaderManager_GetGlobalData();

    // ============ Upload Global Data :: World ============ //
    globalData->time = PerceptronTime;
    SimulatedDateTime *dateTime = World_GetDateTime();
    globalData->timeOfDay = dateTime->normalizedHours;

    // ============ Prepare Global Data :: Camera ============ //
    // Camera Position
    V3 cameraPos_V3 = EC_WPos(ec_camera->component);
    vec3 cameraPos_vec3 = {cameraPos_V3.x, cameraPos_V3.y, cameraPos_V3.z};
    memcpy(globalData->camera_position, cameraPos_vec3, sizeof(vec3));

    V3 forward = V3_NORM(T_Forward(transform));
    V3 up = V3_NORM(T_Up(transform));
    V3 right = V3_NORM(T_Right(transform));

    // Re-orthogonalize (camera might have been rotated)
    right = V3_NORM(V3_CROSS(up, forward));
    up = V3_CROSS(forward, right);

    // Build RIGHT-HANDED view matrix
    // This maps world +Z to view -Z
    mat4 view;

    // Column 0: Right axis (X-axis in view space)
    view[0][0] = right.x;
    view[1][0] = right.y;
    view[2][0] = right.z;
    view[3][0] = -V3_DOT(right, cameraPos_V3);

    // Column 1: Up axis (Y-axis in view space)
    view[0][1] = up.x;
    view[1][1] = up.y;
    view[2][1] = up.z;
    view[3][1] = -V3_DOT(up, cameraPos_V3);

    // Column 2: NEGATIVE Forward axis (Z-axis in view space points BACK)
    // This is the KEY: world +Z forward becomes view -Z forward
    view[0][2] = -forward.x;
    view[1][2] = -forward.y;
    view[2][2] = -forward.z;
    view[3][2] = V3_DOT(forward, cameraPos_V3);

    // Column 3: Homogeneous coordinate
    view[0][3] = 0.0f;
    view[1][3] = 0.0f;
    view[2][3] = 0.0f;
    view[3][3] = 1.0f;

    memcpy(globalData->view, view, sizeof(mat4));

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
}

static void Render_Skybox(EC_Camera *ec_camera)
{
    // Get the skybox shader
    Shader *shader = ec_camera->world->skyboxMaterial->shader;
    // Bind shader
    UseShaderProgram(ec_camera, shader->shaderProgram);
    // Disable depth writing so skybox is always behind everything
    glDepthMask(GL_FALSE);
    // Set depth function to less or equal so skybox renders at max depth
    glDepthFunc(GL_LEQUAL);
    // Disable face culling for skybox (we're inside it)
    glDisable(GL_CULL_FACE);
    // Get skybox mesh
    Mesh *skyboxMesh = ec_camera->world->skyboxMesh;
    // Bind VAO
    glBindVertexArray(skyboxMesh->VAO);
    // Draw skybox
    glDrawElements(GL_TRIANGLES, skyboxMesh->indices_size, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // Re-enable face culling if your engine uses it
    glEnable(GL_CULL_FACE);
    // Re-enable depth writing for regular objects
    glDepthMask(GL_TRUE);
    // Reset depth function to default
    glDepthFunc(GL_LESS);
}

static void Camera_LateUpdate(Component *component)
{
    EC_Camera *ec_camera = component->self;

    // Reset bound shader program tracker
    ec_camera->boundShaderProgram = 0;

    // Hook-up the render target framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, ec_camera->renderTarget->FBO);
    glViewport(0, 0, ec_camera->renderTarget->width, ec_camera->renderTarget->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ============ Prepare Global Data ============ //
    CalculateAndUploadGlobalData(ec_camera);

    // ============ Skybox ============ //
    if (ec_camera->renderSkybox)
    {
        Render_Skybox(ec_camera);
    }
    else
    {
        // Clear to solid color if no skybox
        glClearColor(ec_camera->backgroundColor.r,
                     ec_camera->backgroundColor.g,
                     ec_camera->backgroundColor.b,
                     ec_camera->backgroundColor.a);
    }

    // ============ Mesh Renderers ============ //
    if (ec_camera->renderSolid || ec_camera->renderWireframe)
    {
        Render_MeshRenderers(ec_camera);
    }

    // ============ Debug Colliders ============ //
#ifdef DEBUG_COLLIDERS
    if (ec_camera->renderColliders)
    {
        Camera_RenderDebugColliders(ec_camera);
    }
#endif
    // ============ Render GUIs ============ //
    if (ec_camera->writeGUIToScreen)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (ec_camera->quadVAO != 0 && ec_camera->quadVBO != 0)
        {
            BlitToQuad(ec_camera);
        }
        Render_GUIs(ec_camera);
    }
    else
    {
        Render_GUIs(ec_camera);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (ec_camera->quadVAO != 0 && ec_camera->quadVBO != 0)
        {
            BlitToQuad(ec_camera);
        }
    }
}

// -------------------------
// Blitting
// -------------------------

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

// -------------------------
// Creation & Freeing
// -------------------------

static void EC_Camera_Free(Component *component)
{
    EC_Camera *camera = (EC_Camera *)component->self;
    free(camera);
}

static EC_Camera *EC_Camera_Create(Entity *entity,
                                   V2 viewport,
                                   float fov,
                                   float nearClip,
                                   float farClip,
                                   RenderTarget *renderTarget,
                                   Shader *blitShader,
                                   char *shaderProp_texture_name,
                                   bool createBlitQuad,
                                   bool writeGUIToScreen,
                                   bool renderSkybox,
                                   RGBA backgroundColor)
{
    EC_Camera *ec_camera = malloc(sizeof(EC_Camera));
    ec_camera->viewport = (V2){viewport.x, viewport.y};
    ec_camera->world = World_Get(entity);
    ec_camera->position = &entity->transform.w_pos;
    ec_camera->pixelsPerUnit = 50;
    ec_camera->FOV = fov;
    ec_camera->nearClip = nearClip;
    ec_camera->farClip = farClip;
    ec_camera->writeGUIToScreen = writeGUIToScreen;
    ec_camera->backgroundColor = backgroundColor;
    // Render Modes
#ifdef DEBUG_COLLIDERS
    ec_camera->renderColliders = true;
#endif
    ec_camera->renderWireframe = false;
    ec_camera->renderSolid = true;
    ec_camera->renderSkybox = renderSkybox;
    // Setup render target
    ec_camera->renderTarget = renderTarget;
    if (createBlitQuad)
    {
        SetupBlitQuad(ec_camera);
    }
    else
    {
        ec_camera->quadVAO = 0;
        ec_camera->quadVBO = 0;
    }
    // Load and compile blit shader
    ec_camera->blitShaderProgram = blitShader->shaderProgram;
    // Cache uniform locations
    ec_camera->blitTextureLoc = glGetUniformLocation(ec_camera->blitShaderProgram, shaderProp_texture_name);
    // Culling Mask
    ec_camera->cullingMask = 0xFFFFFFFF; // By default, render all layers
    // Component
    ec_camera->component = Component_Create(ec_camera, entity, EC_T_CAMERA,
                                            EC_Camera_Free, NULL, NULL, NULL,
                                            Camera_LateUpdate, NULL);
    return ec_camera;
}

// -------------------------
// Prefabs
// -------------------------

EC_Camera *Prefab_DisplayCamera(Entity *parent, char *e_name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V2 viewport)
{
    // Entity
    Entity *E_camera = Entity_Create(parent, false, e_name, TS, position, rotation, scale);
    // Setup render target
    RenderTarget *renderTarget = RenderTarget_Create(viewport.x, viewport.y);
    // Load and compile blit shader
    Shader *blitShader = ShaderManager_Get(SHADER_SCREEN_BLIT);
    // Cache uniform locations
    EC_Camera *ec_camera = EC_Camera_Create(E_camera, viewport, Radians(60.0f), 0.1f, 2000.0f, renderTarget, blitShader, "screenTexture", true, true, true, (RGBA){0.0f, 0.0f, 0.0f, 1.0f});
    return ec_camera;
}

EC_Camera *Prefab_RenderTargetCamera(Entity *parent, char *e_name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V2 viewport, RenderTarget *renderTarget, Shader *blitShader, RGBA backgroundColor)
{
    // Entity
    Entity *E_camera = Entity_Create(parent, false, e_name, TS, position, rotation, scale);
    // Create camera
    EC_Camera *ec_camera = EC_Camera_Create(E_camera, viewport, Radians(60.0f), 0.1f, 2000.0f, renderTarget, blitShader, "cameraTexture", false, false, false, backgroundColor);
    return ec_camera;
}

// -------------------------
// Culling Mask
// -------------------------

void EC_Camera_AddToCullingMask(EC_Camera *ec_camera, uint8_t layer)
{
    ec_camera->cullingMask |= (1u << layer);
}

void EC_Camera_RemoveFromCullingMask(EC_Camera *ec_camera, uint8_t layer)
{
    ec_camera->cullingMask &= ~(1u << layer);
}

void EC_Camera_ClearCullingMask(EC_Camera *ec_camera)
{
    ec_camera->cullingMask = 0;
}