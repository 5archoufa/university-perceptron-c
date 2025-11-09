#ifndef MATERIAL_H
#define MATERIAL_H

#include "rendering/shader/shader.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "logging/logger.h"

// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <cglm/cglm.h>

// -------------------------
// Types
// -------------------------

typedef struct Material Material;

struct Material
{
    uint32_t id;
    Shader *shader;
    size_t instanceProps_size;
    /// @brief Array of instance properties for this material ordered by their index in the shader's properties
    ShaderPropertyInstance *instanceProps;
    /// @brief Reference count for shared materials. Do not modify this directly, use Material_AddRef and Material_Release.
    int refCount;
};

// -------------------------
// Creation and Freeing
// -------------------------

Material *Material_Create(Shader *shader, size_t instanceProps_size, ShaderPropertyInstance *instanceProps);
void Material_Free(Material *material);

// -------------------------
// Ref Tracking
// -------------------------

void Material_MarkReferenced(Material *material);
void Material_MarkUnreferenced(Material *material);

// -------------------------
// Setters
// -------------------------

void Material_SetFloat(Material *material, char *name, float value);
void Material_SetVec2(Material *material, char *name, vec2 value);
void Material_SetVec3(Material *material, char *name, vec3 value);
void Material_SetVec4(Material *material, char *name, vec4 value);
void Material_SetInt(Material *material, char *name, int value);
void Material_SetIvec2(Material *material, char *name, ivec2 value);
void Material_SetIvec3(Material *material, char *name, ivec3 value);
void Material_SetIvec4(Material *material, char *name, ivec4 value);
void Material_SetUInt(Material *material, char *name, uint32_t value);
void Material_SetMat2(Material *material, char *name, mat2 value);
void Material_SetMat3(Material *material, char *name, mat3 value);
void Material_SetMat4(Material *material, char *name, mat4 value);
void Material_SetTexture(Material *material, char *name, GLuint textureID);
void Material_SetBool(Material *material, char *name, bool value);

#endif