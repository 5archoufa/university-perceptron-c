#ifndef SHADER_H
#define SHADER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "logging/logger.h"

// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <cglm/cglm.h>

// -------------------------
// Types
// -------------------------

typedef struct Shader Shader;
typedef struct ShaderProperty ShaderProperty;
typedef struct ShaderPropertyInstance ShaderPropertyInstance;
typedef struct ShaderPropBigValue ShaderPropBigValue;
typedef enum ShaderPropertyType ShaderPropertyType;
typedef enum ShaderPropertySource ShaderPropertySource;

typedef union
{
    float floatValue;
    vec2 vec2Value;
    vec3 vec3Value;
    vec4 vec4Value;
    int intValue;
    ivec2 ivec2Value;
    ivec3 ivec3Value;
    ivec4 ivec4Value;
    uint32_t uintValue;
    mat2 mat2Value;
    mat3 mat3Value;
    GLuint sampler2DValue;
} ShaderPropSmallValue;

enum ShaderPropertyType
{
    /// @brief Small Value (4 Bytes)
    MPT_FLOAT,
    /// @brief Small Value (8 Bytes)
    MPT_VEC2,
    /// @brief Small Value (12 Bytes)
    MPT_VEC3,
    /// @brief Small Value (16 Bytes)
    MPT_VEC4,
    /// @brief Small Value (4 Bytes)
    MPT_INT,
    /// @brief Small Value (8 Bytes)
    MPT_IVEC2,
    /// @brief Small Value (12 Bytes)
    MPT_IVEC3,
    /// @brief Small Value (16 Bytes)
    MPT_IVEC4,
    /// @brief Small Value (4 Bytes)
    MPT_UINT,
    /// @brief Small Value (16 Bytes)
    MPT_MAT2,
    /// @brief Small Value (36 Bytes)
    MPT_MAT3,
    /// @brief Small Value (4 Bytes)
    MPT_SAMPLER2D,
    /// @brief Big Value (64 Bytes)
    MPT_MAT4,
};

struct ShaderPropBigValue
{
    void *value;
    size_t size;
};

struct ShaderPropertyInstance
{
    int shaderPropIndex;
    ShaderPropSmallValue smallValue;
    ShaderPropBigValue bigValue;
};

struct ShaderProperty
{
    ShaderPropertyType type;
    GLint loc;
    char *name;
    bool isBig;
    ShaderPropSmallValue smallValue_default;
    ShaderPropSmallValue smallValue_previous;
    ShaderPropBigValue bigValue_default;
    ShaderPropBigValue bigValue_previous;
};

struct Shader
{
    /// @brief Used to lookup shaders
    char *name;
    GLuint shaderProgram;
    // ==== Model Properties ==== //
    mat4 model;
    GLuint modelLoc;
    // ==== Other Properties ==== //
    size_t properties_size;
    ShaderProperty *properties;
};

// -------------------------
// Creation and Freeing
// -------------------------

Shader *Shader_LoadFromFile(char *name, char *vertexPath, char *fragmentPath, size_t properties_size);
Shader *Shader_Create(const char *name, const char *vertexSource, const char *fragmentSource,
                      size_t properties_size);
void Shader_Free(Shader *shader);

// -------------------------
// Property Initializers
// -------------------------
void ShaderProperty_InitDefault_Float(Shader *shader, int propIndex, const char *name, float value);
void ShaderProperty_InitDefault_Vec2(Shader *shader, int propIndex, const char *name, vec2 value);
void ShaderProperty_InitDefault_Vec3(Shader *shader, int propIndex, const char *name, vec3 value);
void ShaderProperty_InitDefault_Vec4(Shader *shader, int propIndex, const char *name, vec4 value);
void ShaderProperty_InitDefault_Int(Shader *shader, int propIndex, const char *name, int value);
void ShaderProperty_InitDefault_IVec2(Shader *shader, int propIndex, const char *name, ivec2 value);
void ShaderProperty_InitDefault_IVec3(Shader *shader, int propIndex, const char *name, ivec3 value);
void ShaderProperty_InitDefault_IVec4(Shader *shader, int propIndex, const char *name, ivec4 value);
void ShaderProperty_InitDefault_UInt(Shader *shader, int propIndex, const char *name, unsigned int value);
void ShaderProperty_InitDefault_Mat2(Shader *shader, int propIndex, const char *name, mat2 value);
void ShaderProperty_InitDefault_Mat3(Shader *shader, int propIndex, const char *name, mat3 value);
void ShaderProperty_InitDefault_Mat4(Shader *shader, int propIndex, const char *name, mat4 value);
void ShaderProperty_InitDefault_Sampler2D(Shader *shader, int propIndex, const char *name, GLuint textureID);
#endif