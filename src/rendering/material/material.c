#include "rendering/material/material.h"
#include "rendering/material/material-manager.h"
#include "rendering/shader/shader.h"
// C
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
// Logging
#include "logging/logger.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <cglm/cglm.h>

static LogConfig _logConfig = {"Material", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Creation and Freeing
// -------------------------

Material *Material_Create(Shader *shader, size_t instanceProps_size, ShaderPropertyInstance *instanceProps)
{
    Material *material = malloc(sizeof(Material));
    material->shader = shader;
    material->id = 0;
    material->refCount = 0;
    material->instanceProps_size = instanceProps_size;
    size_t arraySize = sizeof(ShaderPropertyInstance) * instanceProps_size;
    material->instanceProps = malloc(arraySize);
    memcpy(material->instanceProps, instanceProps, arraySize);
    MaterialManager_RegisterMaterial(material);
    return material;
}

void Material_Free(Material *material)
{
    if(material->refCount > 0) {
        LogError(&_logConfig, "Could not Free Material: RefCount > 0");
        return;
    }
    LogFree(&_logConfig, "");
    free(material->instanceProps);
    free(material);
}

// -------------------------
// Ref Tracking
// -------------------------

void Material_MarkReferenced(Material *material)
{
    material->refCount++;
}

void Material_MarkUnreferenced(Material *material)
{
    material->refCount--;
    if (material->refCount < 0)
    {
        LogError(&_logConfig, "The Material's refCount dropped below zero. This is not supposed to happen, but the material will be freed on the next cleanup cycle regardless.");
        material->refCount = 0;
    }
}

// -------------------------
// Setters
// -------------------------

static ShaderPropertyInstance *Material_AddPropInstance(Material *material, char *name)
{
    Shader *shader = material->shader;
    // Check if it already exists
    for (int i = 0; i < material->instanceProps_size; i++)
    {
        int shaderPropIndex = material->instanceProps[i].shaderPropIndex;
        if (strcmp(shader->properties[shaderPropIndex].name, name) == 0)
        {
            return &material->instanceProps[i];
        }
    }
    // Determine Shader prop Index
    int shaderPropIndex = -1;
    for (int i = 0; i < shader->properties_size; i++)
    {
        if (strcmp(shader->properties[i].name, name) == 0)
        {
            shaderPropIndex = i;
            break;
        }
    }
    if (shaderPropIndex == -1)
    {
        LogError(&_logConfig, "Failed to add instance property '%s' to material, no such property in shader '%s'", name, shader->name);
        return NULL;
    }
    material->instanceProps_size++;
    material->instanceProps = realloc(material->instanceProps, sizeof(ShaderPropertyInstance) * material->instanceProps_size);
    // Rearrange instance properties
    int materialPropIndex = material->instanceProps_size - 1;
    for (; materialPropIndex > 0; materialPropIndex--)
    {
        if (material->instanceProps[materialPropIndex - 1].shaderPropIndex > shaderPropIndex)
        {
            material->instanceProps[materialPropIndex] = material->instanceProps[materialPropIndex - 1];
        }
        else // Insert property instance here
        {
            break;
        }
    }
    material->instanceProps[materialPropIndex].shaderPropIndex = shaderPropIndex;
    material->instanceProps[materialPropIndex].bigValue.size = 0;
    material->instanceProps[materialPropIndex].bigValue.value = NULL;
    return &material->instanceProps[materialPropIndex];
}

void Material_SetFloat(Material *material, char *name, float value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set float property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.floatValue = value;
}

void Material_SetVec2(Material *material, char *name, vec2 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set vec2 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.vec2Value[0] = value[0];
    propInstance->smallValue.vec2Value[1] = value[1];
}

void Material_SetVec3(Material *material, char *name, vec3 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set vec3 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.vec3Value[0] = value[0];
    propInstance->smallValue.vec3Value[1] = value[1];
    propInstance->smallValue.vec3Value[2] = value[2];
}

void Material_SetVec4(Material *material, char *name, vec4 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set vec4 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.vec4Value[0] = value[0];
    propInstance->smallValue.vec4Value[1] = value[1];
    propInstance->smallValue.vec4Value[2] = value[2];
    propInstance->smallValue.vec4Value[3] = value[3];
}

void Material_SetInt(Material *material, char *name, int value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set int property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.intValue = value;
}

void Material_SetIvec2(Material *material, char *name, ivec2 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set ivec2 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.ivec2Value[0] = value[0];
    propInstance->smallValue.ivec2Value[1] = value[1];
}

void Material_SetIvec3(Material *material, char *name, ivec3 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set ivec3 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.ivec3Value[0] = value[0];
    propInstance->smallValue.ivec3Value[1] = value[1];
    propInstance->smallValue.ivec3Value[2] = value[2];
}

void Material_SetIvec4(Material *material, char *name, ivec4 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set ivec4 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.ivec4Value[0] = value[0];
    propInstance->smallValue.ivec4Value[1] = value[1];
    propInstance->smallValue.ivec4Value[2] = value[2];
    propInstance->smallValue.ivec4Value[3] = value[3];
}

void Material_SetUInt(Material *material, char *name, uint32_t value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set uint property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.uintValue = value;
}

void Material_SetMat2(Material *material, char *name, mat2 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set mat2 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    memcpy(propInstance->smallValue.mat2Value, value, sizeof(mat2));
}

void Material_SetMat3(Material *material, char *name, mat3 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set mat3 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    memcpy(propInstance->smallValue.mat3Value, value, sizeof(mat3));
}

void Material_SetMat4(Material *material, char *name, mat4 value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set mat4 property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    if (propInstance->bigValue.value)
    {
        propInstance->bigValue.value = malloc(propInstance->bigValue.size);
    }
    memcpy(propInstance->bigValue.value, value, propInstance->bigValue.size);
}

void Material_SetTexture(Material *material, char *name, GLuint textureID)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set texture property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.sampler2DValue = textureID;
}

void Material_SetBool(Material *material, char *name, bool value)
{
    ShaderPropertyInstance *propInstance = Material_AddPropInstance(material, name);
    if (!propInstance)
    {
        LogError(&_logConfig, "Failed to set bool property '%s' on material, no such property in shader '%s'", name, material->shader->name);
        return;
    }
    propInstance->smallValue.intValue = value ? 1 : 0;
}