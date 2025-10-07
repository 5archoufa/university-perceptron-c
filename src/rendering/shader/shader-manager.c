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
    // ==== Screen Blit Shader ==== //
    Shader *screenBlitShader = Shader_LoadFromFile("Screen-Blit",
                                                   "src/rendering/shader/src/screen-blit/blit_vertex.glsl",
                                                   "src/rendering/shader/src/screen-blit/blit_fragment.glsl",
                                                   1);
    // Screen Texture
    ShaderProperty_InitDefault_Sampler2D(screenBlitShader, 0, "screenTexture", 0);
    // Register Shader
    ShaderManager_AddShader(screenBlitShader);

    // ==== Toon Solid ==== //
    Shader *toonSolidShader = Shader_LoadFromFile("Toon-Solid",
                                                  "src/rendering/shader/src/toon-solid/vertex.glsl",
                                                  "src/rendering/shader/src/toon-solid/fragment.glsl",
                                                  10);
    // Texture
    ShaderProperty_InitDefault_Sampler2D(toonSolidShader, 0, "textureSampler", 0);
    ShaderProperty_InitDefault_Int(toonSolidShader, 1, "useTexture", 0);
    // Toon parameters
    ShaderProperty_InitDefault_Int(toonSolidShader, 2, "toonLevels", 3);
    ShaderProperty_InitDefault_Float(toonSolidShader, 3, "ambientStrength", 0.35f);
    ShaderProperty_InitDefault_Vec3(toonSolidShader, 4, "ambientColor", (vec3){1.0, 1.0, 1.0});
    // Rim
    ShaderProperty_InitDefault_Float(toonSolidShader, 5, "rimThreshold", 0.7f);
    ShaderProperty_InitDefault_Float(toonSolidShader, 6, "rimStrength", 0.25f);
    // Details
    ShaderProperty_InitDefault_Float(toonSolidShader, 7, "detailScale", 10.0f);
    ShaderProperty_InitDefault_Float(toonSolidShader, 8, "detailStrength", 0.1f);
    ShaderProperty_InitDefault_Float(toonSolidShader, 9, "noiseScale", 50.0f);
    // Register Shader
    ShaderManager_AddShader(toonSolidShader);
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

    // Initialize the Model Location


    // Create game shaders at start.
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
    // Check if any slots are free before-hand
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
        if (strcmp(_manager->shaders[i]->name, name) == 0)
        {
            return _manager->shaders[i];
        }
    }
    LogWarning(&_logConfig, "Could not get Shader %s", name);
    return NULL;
}

// -------------------------
// Global Data
// -------------------------

void ShaderManager_UploadGlobalData()
{
    ShaderGlobalData *globalData = ShaderManager_GetGlobalData();
    glBindBuffer(GL_UNIFORM_BUFFER, _manager->globalDataUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderGlobalData), globalData);
}

GLuint ShaderManager_GetModelLoc(){
    return _manager->modelLoc;
}

ShaderGlobalData *ShaderManager_GetGlobalData()
{
    return &_manager->globalData;
}
void ShaderManager_SetGlobal_Camera(mat4 view,
                                    mat4 projection,
                                    vec3 camera_position)
{
    memcpy(_manager->globalData.view, view, sizeof(mat4));
    memcpy(_manager->globalData.projection, projection, sizeof(mat4));
    memcpy(_manager->globalData.camera_position, camera_position, sizeof(vec3));
}
void ShaderManager_SetGlobal_World(float time)
{
    _manager->globalData.time = time;
}
void ShaderManager_SetGlobal_Lighting_Directional(int count,
                                                  vec3 directions[3],
                                                  float intensities[3],
                                                  vec3 colors[3])
{
    _manager->globalData.light_directional_count = count;
    memcpy(_manager->globalData.light_directional_colors, colors, sizeof(vec3) * count);
    memcpy(_manager->globalData.light_directional_directions, directions, sizeof(vec3) * count);
    memcpy(_manager->globalData.light_directional_intensities, intensities, sizeof(float) * count);
}
void ShaderManager_SetGlobal_Lighting_Point(int count,
                                            vec3 colors[8],
                                            vec3 positions[8],
                                            float intensities[8],
                                            float ranges[8])
{
    _manager->globalData.light_point_count = count;
    memcpy(_manager->globalData.light_point_colors, colors, sizeof(vec3) * count);
    memcpy(_manager->globalData.light_point_positions, positions, sizeof(vec3) * count);
    memcpy(_manager->globalData.light_point_intensities, intensities, sizeof(float) * count);
    memcpy(_manager->globalData.light_point_ranges, ranges, sizeof(float) * count);
}