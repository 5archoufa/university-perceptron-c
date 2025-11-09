#include "rendering/texture/texture-manager.h"
#include "rendering/shader/shader-manager.h"
#include "logging/logger.h"

#include <stdlib.h>

static LogConfig _logConfig = {"ShaderManager", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static ShaderManager *_manager = NULL;

// -------------------------
// Creation and Freeing
// -------------------------

static void ShaderManager_CreateGameShaders()
{
    // ============ Screen Blit ============ //
    Shader *screenBlitShader = Shader_LoadFromFile(SHADER_SCREEN_BLIT,
                                                   "src/rendering/shader/src/screen-blit/vertex.glsl",
                                                   "src/rendering/shader/src/screen-blit/fragment.glsl",
                                                   1);
    // Screen Texture
    ShaderProperty_InitDefault_Sampler2D(screenBlitShader, 0, "screenTexture", 0);

    // ============ Quad Blit ============ //
    Shader *quadBlitShader = Shader_LoadFromFile(SHADER_QUAD_BLIT,
                                                 "src/rendering/shader/src/quad-blit/vertex.glsl",
                                                 "src/rendering/shader/src/quad-blit/fragment.glsl",
                                                 1);
    ShaderProperty_InitDefault_Sampler2D(quadBlitShader, 0, "cameraTexture", 0);

    // ============ Island ============ //
    Shader *islandShader = Shader_LoadFromFile(SHADER_ISLAND,
                                               "src/rendering/shader/src/island/vertex.glsl",
                                               "src/rendering/shader/src/island/fragment.glsl",
                                               0);

    // ==== Toon Solid ==== //
    Shader *toonSolidShader = Shader_LoadFromFile(SHADER_TOON_SOLID,
                                                  "src/rendering/shader/src/toon/vertex.glsl",
                                                  "src/rendering/shader/src/toon/fragment.glsl",
                                                  0);

    // ============ Sea ============ //
    Shader *seaShader = Shader_LoadFromFile(SHADER_SEA,
                                            "src/rendering/shader/src/sea/vertex.glsl",
                                            "src/rendering/shader/src/sea/fragment.glsl",
                                            0);

    // ============ UI Text ============ //
    Shader *uiTextShader = Shader_LoadFromFile(SHADER_UI_TEXT,
                                               "src/rendering/shader/src/ui-text/vertex.glsl",
                                               "src/rendering/shader/src/ui-text/fragment.glsl",
                                               2);
    ShaderProperty_InitDefault_Sampler2D(uiTextShader, 0, "uTexture", 0);
    ShaderProperty_InitDefault_Vec4(uiTextShader, 1, "uColor", (vec4){1.0f, 1.0f, 1.0f, 1.0f});

    // ============ Skybox ============ //
    Shader *skyboxShader = Shader_LoadFromFile(SHADER_SKYBOX,
                                               "src/rendering/shader/src/skybox/vertex.glsl",
                                               "src/rendering/shader/src/skybox/fragment.glsl",
                                               0);

    // ============ Sprite ============ //

    Shader *spriteShader = Shader_LoadFromFile(SHADER_SPRITE,
                                               "src/rendering/shader/src/sprite/vertex.glsl",
                                               "src/rendering/shader/src/sprite/fragment.glsl",
                                               3);
    ShaderProperty_InitDefault_Sampler2D(spriteShader, 0, "textureSampler", 0);
    ShaderProperty_InitDefault_Vec4(spriteShader, 1, "color", (vec4){1.0f, 1.0f, 1.0f, 1.0f});
    ShaderProperty_InitDefault_Int(spriteShader, 2, "useTexture", 0);

    // ============ Trigle Page ============ //
    Shader *triglePageShader = Shader_LoadFromFile(SHADER_TRIGLE_PAGE,
                                                   "src/rendering/shader/src/trigle-page/vertex.glsl",
                                                   "src/rendering/shader/src/trigle-page/fragment.glsl",
                                                   1);
    ShaderProperty_InitDefault_Sampler2D(triglePageShader, 0, "cameraTexture", 0);

#ifdef DEBUG_COLLIDERS
    // ============ Debug Collider ============ //
    Shader *debugColliderShader = Shader_LoadFromFile(SHADER_DEBUG_COLLIDER,
                                                      "src/rendering/shader/src/debug-collider/vertex.glsl",
                                                      "src/rendering/shader/src/debug-collider/fragment.glsl",
                                                      5);
    // Edge thickness
    ShaderProperty_InitDefault_Float(debugColliderShader, 0, "edgeThickness", 0.02f);
    // Edge color (white)
    ShaderProperty_InitDefault_Vec3(debugColliderShader, 1, "edgeColor", (vec3){1.0f, 1.0f, 1.0f});
    // Face color (red)
    ShaderProperty_InitDefault_Vec3(debugColliderShader, 2, "faceColor", (vec3){1.0f, 0.2f, 0.2f});
    // Face alpha
    ShaderProperty_InitDefault_Float(debugColliderShader, 3, "faceAlpha", 0.3f);
    // Noise scale
    ShaderProperty_InitDefault_Float(debugColliderShader, 4, "noiseScale", 8.0f);
#endif
}

ShaderManager *ShaderManager_Create()
{
    ShaderManager *manager = malloc(sizeof(ShaderManager));
    manager->shaders_size = 0;
    manager->shaders = NULL;

    // Pre-allocate some space
    manager->shaders_size = 10;
    manager->shaders = malloc(sizeof(Shader *) * manager->shaders_size);
    for (int i = 0; i < manager->shaders_size; i++)
    {
        manager->shaders[i] = NULL;
    }

    // Initialize the Shader Global Data UBO
    glGenBuffers(1, &manager->globalDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, manager->globalDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderGlobalData), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, manager->globalDataUBO); // binding = 0 (Acts like a socket, all shaders listening to that have access to the data)

    // Create game shaders at start.
    ShaderManager_Select(manager);
    ShaderManager_CreateGameShaders();

    return manager;
}

void ShaderManager_Free(ShaderManager *manager)
{
    if (!manager)
    {
        return;
    }
    for (int i = 0; i < manager->shaders_size; i++)
    {
        if (!manager->shaders[i])
        {
            continue;
        }
        Shader_Free(manager->shaders[i]);
    }
    free(manager->shaders);
    free(manager);
}

// -------------------------
// Functions
// -------------------------

void ShaderManager_Select(ShaderManager *manager)
{
    _manager = manager;
}

void ShaderManager_AddShader(Shader *shader)
{
    for (int i = 0; i < _manager->shaders_size; i++)
    {
        if (_manager->shaders[i] == NULL)
        {
            _manager->shaders[i] = shader;
            return;
        }
    }
    _manager->shaders_size++;
    _manager->shaders = realloc(_manager->shaders, sizeof(Shader *) * _manager->shaders_size);
    _manager->shaders[_manager->shaders_size - 1] = shader;
}

Shader *ShaderManager_Get(const char *name)
{
    for (int i = 0; i < _manager->shaders_size; i++)
    {
        if (_manager->shaders[i] == NULL)
            continue;
        if (strcmp(_manager->shaders[i]->name, name) == 0)
        {
            return _manager->shaders[i];
        }
    }
    LogError(&_logConfig, "Could not get Shader %s", name);
    return NULL;
}

// -------------------------
// Global Data
// -------------------------

void ShaderManager_UploadGlobalData()
{
    ShaderGlobalData *globalData = ShaderManager_GetGlobalData();
    // Log(&_logConfig, "---- Uploading Global Shader Data to UBO ---- ");
    // Log(&_logConfig, "Camera Position: %f, %f, %f", globalData->camera_position[0], globalData->camera_position[1], globalData->camera_position[2]);
    // Log(&_logConfig, "Camera View Matrix:");
    // for (int i = 0; i < 4; i++)
    // {
    //     Log(&_logConfig, "  %f, %f, %f, %f", globalData->view[i][0], globalData->view[i][1], globalData->view[i][2], globalData->view[i][3]);
    // }
    // Log(&_logConfig, "Camera Projection Matrix:");
    // for (int i = 0; i < 4; i++)
    // {
    //     Log(&_logConfig, "  %f, %f, %f, %f", globalData->projection[i][0], globalData->projection[i][1], globalData->projection[i][2], globalData->projection[i][3]);
    // }
    // Log(&_logConfig, "Time: %f", globalData->time);
    // Log(&_logConfig, "Directional Lights: %d", globalData->light_directional_count);
    // for (int i = 0; i < globalData->light_directional_count; i++)
    // {
    //     Log(&_logConfig, " - Dir %d: Dir(%f, %f, %f), Color(%f, %f, %f), Intensity(%f)", i,
    //         globalData->light_directional_directions[i][0],
    //         globalData->light_directional_directions[i][1],
    //         globalData->light_directional_directions[i][2],
    //         globalData->light_directional_colors[i][0],
    //         globalData->light_directional_colors[i][1],
    //         globalData->light_directional_colors[i][2],
    //         globalData->light_directional_colors[i][3]);
    // }
    // Log(&_logConfig, "Point Lights: %d", globalData->light_point_count);
    // for (int i = 0; i < globalData->light_point_count; i++)
    // {
    //     Log(&_logConfig, " - Point %d: Pos(%f, %f, %f), Color(%f, %f, %f), Intensity(%f), Range(%f)", i,
    //         globalData->light_point_positions[i][0],
    //         globalData->light_point_positions[i][1],
    //         globalData->light_point_positions[i][2],
    //         globalData->light_point_colors[i][0],
    //         globalData->light_point_colors[i][1],
    //         globalData->light_point_colors[i][2],
    //         globalData->light_point_colors[i][3],
    //         globalData->light_point_positions[i][3]);
    // }
    glBindBuffer(GL_UNIFORM_BUFFER, _manager->globalDataUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderGlobalData), globalData);
}

ShaderGlobalData *ShaderManager_GetGlobalData()
{
    return &_manager->globalData;
}