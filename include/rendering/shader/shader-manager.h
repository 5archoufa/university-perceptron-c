#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "rendering/shader/shader.h"

// -------------------------
// Constants
// -------------------------
// Limits
#define LIGHT_DIRECT_COUNT_MAX 3
#define LIGHT_POINT_COUNT_MAX 8
// Shaders
#define SHADER_SCREEN_BLIT "Screen-Blit"
#define SHADER_TOON_SOLID "Toon-Solid"

// -------------------------
// Types
// -------------------------

typedef struct ShaderGlobalData{
    // ==== Camera ==== //
    mat4 view;
    mat4 projection;
    vec3 camera_position;
    // ==== World ==== //
    float time;
    // ==== Lighting ==== //
    // Directional Lights
    int light_directional_count;
    vec3 light_directional_directions[LIGHT_DIRECT_COUNT_MAX];
    float light_directional_intensities[LIGHT_DIRECT_COUNT_MAX];
    vec3 light_directional_colors[LIGHT_DIRECT_COUNT_MAX];
    // Point Lights
    int light_point_count;
    vec3 light_point_colors[LIGHT_POINT_COUNT_MAX];
    vec3 light_point_positions[LIGHT_POINT_COUNT_MAX];
    float light_point_intensities[LIGHT_POINT_COUNT_MAX];
    float light_point_ranges[LIGHT_POINT_COUNT_MAX];
} ShaderGlobalData;

typedef struct ShaderManager{
    ShaderGlobalData globalData;
    GLuint globalDataUBO;
    GLuint modelLoc;
    size_t shaders_size;
    Shader **shaders;
} ShaderManager;

// -------------------------
// Creation and Freeing
// -------------------------

ShaderManager *ShaderManager_Create();
void ShaderManager_Free(ShaderManager *manager);

// -------------------------
// Functions
// -------------------------

void ShaderManager_Select(ShaderManager* manager);
void ShaderManager_AddShader(Shader *shader);
Shader *ShaderManager_Get(const char *name);

// -------------------------
// Global Data
// -------------------------

void ShaderManager_UploadGlobalData();
GLuint ShaderManager_GetModelLoc();
ShaderGlobalData* ShaderManager_GetGlobalData();
void ShaderManager_SetGlobal_Camera(mat4 view,
                                    mat4 projection,
                                    vec3 camera_position);
void ShaderManager_SetGlobal_World(float time);
void ShaderManager_SetGlobal_Lighting_Directional(int light_directional_count,
                                                  vec3 light_directional_directions[3],
                                                  float light_directional_intensities[3],
                                                  vec3 light_directional_colors[3]);
void ShaderManager_SetGlobal_Lighting_Point(int light_point_count,
                                            vec3 light_point_colors[8],
                                            vec3 light_point_positions[8],
                                            float light_point_intensities[8],
                                            float light_point_ranges[8]);

#endif