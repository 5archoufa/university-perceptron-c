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
typedef enum ShaderPropertyType ShaderPropertyType;

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
    MPT_FLOAT,
    MPT_VEC2,
    MPT_VEC3,
    MPT_VEC4,
    MPT_INT,
    MPT_IVEC2,
    MPT_IVEC3,
    MPT_IVEC4,
    MPT_UINT,
    MPT_MAT2,
    MPT_MAT3,
    MPT_MAT4,
    MPT_SAMPLER2D
};

struct ShaderPropertyInstance
{
    int shaderPropIndex;
    ShaderPropSmallValue smallValue;
    void *bigValue;
};

struct ShaderProperty
{
    ShaderPropertyType type;
    GLint loc;
    char *name;
    /// @brief Indicates whether or not a value different from the default has been uploaded previously
    void *bigValue_previous;
    ShaderPropSmallValue smallValue_previous;
    bool isBig;
    ShaderPropSmallValue smallValue_default;
    void *bigValue_default;
};

struct Shader
{
    /// @brief Used to lookup shaders
    char *name;
    GLuint shaderProgram;
    size_t properties_size;
    ShaderProperty *properties;
};

// -------------------------
// Creation and Freeing
// -------------------------

Shader *Shader_Create(const char *name, const char *vertexSource, const char *fragmentSource,
                      size_t properties_size, ShaderProperty *properties);
void Shader_Free(Shader *shader);

// -------------------------
// Property Initializers
// -------------------------

void ShaderProperty_InitDefault_Float(ShaderProperty *prop, GLuint program, const char *name, float value);
void ShaderProperty_InitDefault_Vec2(ShaderProperty *prop, GLuint program, const char *name, vec2 value);
void ShaderProperty_InitDefault_Vec3(ShaderProperty *prop, GLuint program, const char *name, vec3 value);
void ShaderProperty_InitDefault_Vec4(ShaderProperty *prop, GLuint program, const char *name, vec4 value);
void ShaderProperty_InitDefault_Int(ShaderProperty *prop, GLuint program, const char *name, int value);
void ShaderProperty_InitDefault_IVec2(ShaderProperty *prop, GLuint program, const char *name, ivec2 value);
void ShaderProperty_InitDefault_IVec3(ShaderProperty *prop, GLuint program, const char *name, ivec3 value);
void ShaderProperty_InitDefault_IVec4(ShaderProperty *prop, GLuint program, const char *name, ivec4 value);
void ShaderProperty_InitDefault_UInt(ShaderProperty *prop, GLuint program, const char *name, unsigned int value);
void ShaderProperty_InitDefault_Mat2(ShaderProperty *prop, GLuint program, const char *name, mat2 value);
void ShaderProperty_InitDefault_Mat3(ShaderProperty *prop, GLuint program, const char *name, mat3 value);
void ShaderProperty_InitDefault_Mat4(ShaderProperty *prop, GLuint program, const char *name, mat4 value);
void ShaderProperty_InitDefault_Sampler2D(ShaderProperty *prop, GLuint program, const char *name, GLuint textureID);

#endif