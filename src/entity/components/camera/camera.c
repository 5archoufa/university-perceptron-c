#include "entity/components/camera/camera.h"
#include <stdlib.h>
#include <stdint.h>
#include "ui/window.h"
#include "entity/entity.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/renderer/bounds.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include "perceptron.h"
#include <math.h>
#include "utilities/math/stupid_math.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
#include "utilities/file/file.h"
#include "rendering/shader/shader.h"
#include "rendering/material/material.h"
#include "entity/components/lighting/ls_directional_light.h"
#include "logging/logger.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

const int CAMERA_RENDER_MODE_COUNT = 3;
static LogConfig _logConfig = {"EC_Camera", LOG_LEVEL_WARN, LOG_COLOR_BLUE};

// Forward declarations
static void BlitToScreen(GLuint textureID, GLuint shaderProgram, GLuint quadVAO);

static void RenderRulers(EC_Camera *ec_camera)
{
    uint32_t *pixels = (uint32_t *)ec_camera->image->data;
    int width = ec_camera->image->width;
    int height = ec_camera->image->height;
    // Edges
    for (int x = 0; x < ec_camera->viewport.x; x++)
    {
        pixels[0 * width + x] = PIXEL_GREEN;
        pixels[(height - 1) * width + x] = PIXEL_GREEN;
    }
    for (int y = 0; y < ec_camera->viewport.y; y++)
    {
        pixels[y * width + 0] = PIXEL_GREEN;
        pixels[y * width + (width - 1)] = PIXEL_GREEN;
    }

    // Center
    for (int x = 0; x < ec_camera->viewport.x; x++)
    {
        pixels[(height / 2) * width + x] = PIXEL_GREEN;
    }
    for (int y = 0; y < ec_camera->viewport.y; y++)
    {
        pixels[y * width + (width / 2)] = PIXEL_GREEN;
    }
}

static void Camera_Render3D_Wire(EC_Camera *ec_camera, Rect rect, uint32_t color)
{
    uint32_t *pixels = (uint32_t *)ec_camera->image->data;
    int cameraWidth = ec_camera->image->width;
    for (int y = rect.start.y; y <= rect.end.y; y++)
    {
        pixels[y * cameraWidth + rect.start.x] = color;
    }
    for (int y = rect.start.y; y <= rect.end.y; y++)
    {
        pixels[y * cameraWidth + rect.end.x] = color;
    }
    for (int x = rect.start.x; x <= rect.end.x; x++)
    {
        pixels[rect.start.y * cameraWidth + x] = color;
    }
    for (int x = rect.start.x; x <= rect.end.x; x++)
    {
        pixels[rect.end.y * cameraWidth + x] = color;
    }
}

const vec3 VEC_UP = {0.0f, 1.0f, 0.0f};
const vec3 VEC_RIGHT = {1.0f, 0.0f, 0.0f};
const vec3 VEC_FORWARD = {0.0f, 0.0f, -1.0f};

static void Camera_Render_OpenGL(EC_Camera *ec_camera, V3 camPos, EC_Renderer3D *ec_renderer3d, mat4 view)
{

    void Camera_Render_OpenGL(EC_Camera * ec_camera, EC_Renderer3D * ec_renderer3d)
    {
        Material *material = ec_renderer3d->material;
        Shader *shader = material->shader;
        // Switch shader if necessary
        if (ec_camera->boundShaderProgram != shader->shaderProgram)
        {
            glUseProgram(shader->shaderProgram);
            ec_camera->boundShaderProgram = shader->shaderProgram;
        }
        int j = 0; // Index of the latest instance property inside material->instanceProps
        ShaderProperty *prop;
        void *bigValue;
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
                shader->properties[i].bigValue_previous = bigValue;
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
                    glUniformMatrix2fv(shader->properties[i].loc, 1, GL_FALSE, smallValue.mat2Value);
                    break;
                case MPT_MAT3:
                    glUniformMatrix3fv(shader->properties[i].loc, 1, GL_FALSE, smallValue.mat3Value);
                    break;
                default:
                    LogError(&_logConfig, "Error: Unsupported small property type %d for property '%s'", prop->type, prop->name);
                    break;
                }
            }
        }
    }

    Entity *entity = ec_renderer3d->component->entity;
    Mesh *mesh = ec_renderer3d->mesh;

    // Use the material's shader program
    Material *material = ec_renderer3d->material;
    glUseProgram(material->shaderProgram);

    // Fetch the model matrix
    mat4 model;
    T_WMatrix(&entity->transform, model);

    // Upload Matrices
    glUniform3f(ec_camera->viewPosLoc, camPos.x, camPos.y, camPos.z);
    glUniformMatrix4fv(ec_camera->modelLoc, 1, GL_FALSE, (float *)model);
    glUniformMatrix4fv(ec_camera->viewLoc, 1, GL_FALSE, (float *)view);
    glUniformMatrix4fv(ec_camera->projLoc, 1, GL_FALSE, (float *)ec_camera->proj);

    // Bind Texture
    bool useTexture = (ec_renderer3d->texture != NULL && ec_renderer3d->texture->textureID != 0);
    if (useTexture)
    {
        glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, ec_renderer3d->texture->textureID);
        glUniform1i(ec_camera->textureLoc, 0);
    }
    glUniform1i(ec_camera->useTextureLoc, useTexture ? 1 : 0);

    // Draw
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
    // Clear with a distinct color to show render target boundaries
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Dark blue-gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    // Activate 3D shader BEFORE rendering
    glUseProgram(ShaderProgram);

    //// Build view matrix ////
    V3 cameraPos = EC_WPos(ec_camera->component);
    mat4 view;
    V3 f = T_Forward(&ec_camera->component->entity->transform);
    V3 r = V3_NORM(V3_CROSS(WORLD_UP, f));
    V3 u = V3_CROSS(f, r);
    // Build look-at manually
    vec3 eye = {cameraPos.x, cameraPos.y, cameraPos.z};
    vec3 center = {eye[0] + f.x, eye[1] + f.y, eye[2] + f.z};
    vec3 up = {u.x, u.y, u.z};
    glm_lookat(eye, center, up, view);

    // Flip the camera to look toward +Z instead of -Z
    // mat4 flipZ = GLM_MAT4_IDENTITY_INIT;
    // flipZ[2][2] = -1.0f;
    // glm_mat4_mul(flipZ, view, view);

    //// Set lighting uniforms ////
    // Directional Lights
    int lights_directional_size = ec_camera->world->lights_directional_size;
    float lights_directional_intensity[lights_directional_size];
    float lights_directional_color[lights_directional_size * 3];
    float lights_directional_dir[lights_directional_size * 3];
    glUniform1i(ec_camera->numDirLightsLoc, lights_directional_size);
    // Upload directional lights data
    for (int i = 0; i < lights_directional_size; i++)
    {
        EC_Light *ec_light = ec_camera->world->lights[i];
        LS_DirectionalLight *dirLight = (LS_DirectionalLight *)ec_light->lightSource;
        lights_directional_intensity[i] = dirLight->intensity;
        lights_directional_color[i * 3 + 0] = ((dirLight->color >> 16) & 0xFF) / 255.0f;
        lights_directional_color[i * 3 + 1] = ((dirLight->color >> 8) & 0xFF) / 255.0f;
        lights_directional_color[i * 3 + 2] = (dirLight->color & 0xFF) / 255.0f;
        // Calculate light direction vector based on ec_light->eulerAngles
        V3 lightDir = EC_Forward(ec_light->component);
        LogWarning(&_logConfig, "Dire light forward: (%.2f, %.2f, %.2f)", lightDir.x, lightDir.y, lightDir.z);
        lightDir = V3_SCALE(lightDir, -1.0f);
        lightDir = V3_NORM(lightDir);
        lights_directional_dir[i * 3 + 0] = lightDir.x;
        lights_directional_dir[i * 3 + 1] = lightDir.y;
        lights_directional_dir[i * 3 + 2] = lightDir.z;
        // printf("Dir Light %d: dir(%.2f, %.2f, %.2f), color(%.2f, %.2f, %.2f), intensity(%.2f)\n",
        //        i,
        //        lights_directional_dir[i * 3 + 0], lights_directional_dir[i * 3 + 1], lights_directional_dir[i * 3 + 2],
        //        lights_directional_color[i * 3 + 0], lights_directional_color[i * 3 + 1], lights_directional_color[i * 3 + 2],
        //        lights_directional_intensity[i]);
    }
    glUniform3fv(ec_camera->dirLightDirLoc, lights_directional_size, (float *)lights_directional_dir);
    glUniform3fv(ec_camera->dirLightColorLoc, lights_directional_size, (float *)lights_directional_color);
    glUniform1fv(ec_camera->dirLightIntensityLoc, lights_directional_size, lights_directional_intensity);
    // Point Lights
    glUniform1i(ec_camera->numPointLightsLoc, 0);

    // Render 3D models
    int rendererCount_3D = ec_camera->world->rendererCount_3D;
    EC_Renderer3D **renderers = ec_camera->world->renderers_3D;
    for (int i = 0; i < rendererCount_3D; i++)
    {
        Camera_Render_OpenGL(ec_camera, cameraPos, renderers[i], view);
    }
    // Blit to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // BlitToScreen will handle viewport scaling and aspect ratio preservation
    BlitToScreen(ec_camera->renderTarget->colorTexture, ec_camera->blitShaderProgram, ec_camera->quadVAO);
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

void BlitToScreen(GLuint textureID, GLuint shaderProgram, GLuint quadVAO)
{
    // Bind default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    // Calculate aspect ratios to maintain proper scaling
    float renderTargetAspect = (float)WindowConfig.imageWidth / (float)WindowConfig.imageHeight;
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

    glUseProgram(shaderProgram);
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, SCREEN_TEXTURE), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

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

EC_Camera *EC_Camera_Create(Entity *entity, XImage *image, V2 viewport, float fov,
                            float nearClip, float farClip)
{
    EC_Camera *ec_camera = malloc(sizeof(EC_Camera));
    ec_camera->viewport = (V2){viewport.x, viewport.y};
    ec_camera->image = image;
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
    char *blitVertexSource = File_LoadStr("src/shader/blit_vertex.glsl");
    char *blitFragmentSource = File_LoadStr("src/shader/blit_fragment.glsl");
    if (!blitVertexSource || !blitFragmentSource)
    {
        printf("ERROR: Failed to load blit shader files\n");
        if (blitVertexSource)
            free(blitVertexSource);
        if (blitFragmentSource)
            free(blitFragmentSource);
        free(ec_camera);
        return NULL;
    }
    ec_camera->blitShaderProgram = Shader_CreateShaderProgram(blitVertexSource, blitFragmentSource);
    free(blitVertexSource);
    free(blitFragmentSource);

    // Cache uniform locations
    ec_camera->modelLoc = glGetUniformLocation(ShaderProgram, "model");
    ec_camera->viewLoc = glGetUniformLocation(ShaderProgram, "view");
    ec_camera->projLoc = glGetUniformLocation(ShaderProgram, "projection");
    ec_camera->textureLoc = glGetUniformLocation(ShaderProgram, "textureSampler");
    ec_camera->useTextureLoc = glGetUniformLocation(ShaderProgram, "useTexture");
    ec_camera->viewPosLoc = glGetUniformLocation(ShaderProgram, "viewPos");
    ec_camera->numDirLightsLoc = glGetUniformLocation(ShaderProgram, "numDirLights");
    ec_camera->dirLightDirLoc = glGetUniformLocation(ShaderProgram, "dirLightDirections");
    ec_camera->dirLightColorLoc = glGetUniformLocation(ShaderProgram, "dirLightColors");
    ec_camera->dirLightIntensityLoc = glGetUniformLocation(ShaderProgram, "dirLightIntensities");
    ec_camera->numPointLightsLoc = glGetUniformLocation(ShaderProgram, "numPointLights");
    ec_camera->pointLightPosLoc = glGetUniformLocation(ShaderProgram, "pointLightPositions");
    ec_camera->pointLightColorLoc = glGetUniformLocation(ShaderProgram, "pointLightColors");
    ec_camera->pointLightIntensityLoc = glGetUniformLocation(ShaderProgram, "pointLightIntensities");
    ec_camera->pointLightRangeLoc = glGetUniformLocation(ShaderProgram, "pointLightRanges");

    ec_camera->component = Component_Create(ec_camera, entity, EC_T_CAMERA,
                                            EC_Camera_Free, NULL, NULL, NULL,
                                            Camera_LateUpdate, NULL);
    // Pre-calculate projection matrix
    glm_perspective(ec_camera->FOV,
                    ec_camera->viewport.x / ec_camera->viewport.y,
                    ec_camera->nearClip,
                    ec_camera->farClip,
                    ec_camera->proj);
    return ec_camera;
}

void EC_Camera_Free(Component *component)
{
    EC_Camera *camera = (EC_Camera *)component->self;
    free(camera);
}