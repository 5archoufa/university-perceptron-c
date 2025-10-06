#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "rendering/shader/shader.h"

// -------------------------
// Types
// -------------------------

typedef struct ShaderManager ShaderManager;
typedef struct Shader Shader;

struct ShaderManager{
    size_t shaders_size;
    Shader **shaders;
};

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

#endif