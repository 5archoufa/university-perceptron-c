#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "rendering/shader/shader.h"

// -------------------------
// Constants
// -------------------------
// Limits
#define LIGHT_DIRECT_COUNT_MAX 4
#define LIGHT_POINT_COUNT_MAX 8
// Shaders
#define SHADER_SCREEN_BLIT "Screen-Blit"
#define SHADER_TOON_SOLID "Toon Solid"
#define SHADER_SEA "Sea"

// -------------------------
// Types
// -------------------------

typedef struct ShaderGlobalData{
    // ==== Camera ==== //
    mat4 view; // 64 bytes
    mat4 projection; // 64 bytes
    vec4 camera_position; // 16 bytes
    // ==== World ==== //
    float time; // 4 bytes
    float _pad_world_1; // 4 bytes
    float _pad_world_2; // 4 bytes
    float _pad_world_3; // 4 bytes
    // ==== Lighting ==== //
    // Directional Lights
    int light_directional_count; // 4 bytes
    float _pad_light_dir_1; // 4 bytes
    float _pad_light_dir_2; // 4 bytes
    float _pad_light_dir_3; // 4 bytes
    vec4 light_directional_directions[LIGHT_DIRECT_COUNT_MAX]; // [0..2] is direction, [3] is padding
    vec4 light_directional_colors[LIGHT_DIRECT_COUNT_MAX]; // [0..2] is color, [3] is intensity
    // Point Lights
    int light_point_count; // 4 bytes
    float _pad_light_point_1; // 4 bytes
    float _pad_light_point_2; // 4 bytes
    float _pad_light_point_3; // 4 bytes
    vec4 light_point_colors[LIGHT_POINT_COUNT_MAX]; // [0..2] is color, [3] is intensity
    vec4 light_point_positions[LIGHT_POINT_COUNT_MAX]; // [0..2] is position, [3] is range
} ShaderGlobalData;

typedef struct ShaderManager{
    ShaderGlobalData globalData;
    GLuint globalDataUBO;
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
ShaderGlobalData* ShaderManager_GetGlobalData();

#endif