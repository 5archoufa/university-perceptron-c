#include "rendering/shader/shader.h"
#include "rendering/shader/shader-manager.h"
#include "logging/logger.h"
#include "utilities/file/file.h"
#include <cglm/cglm.h>
#include <stdlib.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------------
// Utilities
// -------------------------

static LogConfig _logConfig = {"Shader", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

inline static GLuint Shader_CreateShaderProgram(const char *vertSrc, const char *fragSrc)
{
    GLuint vs = Shader_Compile(GL_VERTEX_SHADER, vertSrc);
    GLuint fs = Shader_Compile(GL_FRAGMENT_SHADER, fragSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info[512];
        glGetProgramInfoLog(program, 512, NULL, info);
        LogError(&_logConfig, "Program linking error: %s\n", info);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    LogSuccess(&_logConfig, "Shader program created successfully.\n");
    return program;
}

inline static GLuint Shader_Compile(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        fprintf(stderr, "Shader compilation error: %s\n", info);
    }
    return shader;
}

inline static void ShaderProperty_PartialInit(ShaderProperty *prop, char *name, GLint loc, ShaderPropertyType type, bool isBig)
{
    prop->name = malloc(sizeof(char) * 64);
    strcpy(prop->name, name);
    prop->loc = loc;
    prop->type = type;
    prop->isBig = isBig;
    prop->bigValue_default = NULL;
    prop->bigValue_previous = NULL;
}

// -------------------------
// Creation and Freeing
// -------------------------

void Shader_Create(char *name, char *vertexSource, char *fragmentSource, size_t properties_size, ShaderProperty *properties)
{
    GLint shaderProgram = Shader_CreateShaderProgram(vertexSource, fragmentSource);
    if (shaderProgram == 0)
    {
        LogError(&_logConfig, "Error Creating Shader %s", name);
    }

    Shader *shader = malloc(sizeof(Shader));
    shader->shaderProgram = shaderProgram;
    shader->name = malloc(sizeof(char) * 64);
    strcpy(shader->name, name);
    shader->properties_size = properties_size;
    shader->properties = malloc(sizeof(ShaderProperty) * properties_size);
    for (int i = 0; i < properties_size; i++)
    {
        shader->properties[i] = properties[i];
    }

    // Add to shader list
    ShaderManager_AddShader(shader);
}

void Shader_Free(Shader *shader)
{
    glDeleteProgram(shader->shaderProgram);
    for (int i = 0; i < shader->properties_size; i++)
    {
        free(shader->properties[i].name);
        if (shader->properties[i].isBig)
        {
            free(shader->properties[i].bigValue_default);
            free(shader->properties[i].bigValue_previous);
        }
    }
    free(shader->name);
    free(shader->properties);
    free(shader);
}

// -------------------------
// Property Initializers
// -------------------------

void ShaderProperty_InitDefault_Float(ShaderProperty *prop, GLuint program, char *name, float value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_FLOAT, false);
    prop->smallValue_default.floatValue = value;
}

void ShaderProperty_InitDefault_Vec2(ShaderProperty *prop, GLuint program, char *name, vec2 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_VEC2, false);
    memcpy(&prop->smallValue_default.vec2Value, value, sizeof(vec2));
}

void ShaderProperty_InitDefault_Vec3(ShaderProperty *prop, GLuint program, char *name, vec3 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_VEC3, false);
    memcpy(&prop->smallValue_default.vec3Value, value, sizeof(vec3));
}

void ShaderProperty_InitDefault_Vec4(ShaderProperty *prop, GLuint program, char *name, vec4 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_VEC4, false);
    memcpy(&prop->smallValue_default.vec4Value, value, sizeof(vec4));
}

void ShaderProperty_InitDefault_Int(ShaderProperty *prop, GLuint program, char *name, int value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_INT, false);
    prop->smallValue_default.intValue = value;
}

void ShaderProperty_InitDefault_IVec2(ShaderProperty *prop, GLuint program, char *name, ivec2 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_IVEC2, false);
    memcpy(&prop->smallValue_default.ivec2Value, value, sizeof(ivec2));
}

void ShaderProperty_InitDefault_IVec3(ShaderProperty *prop, GLuint program, char *name, ivec3 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_IVEC3, false);
    memcpy(&prop->smallValue_default.ivec3Value, value, sizeof(ivec3));
}

void ShaderProperty_InitDefault_IVec4(ShaderProperty *prop, GLuint program, char *name, ivec4 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_IVEC4, false);
    memcpy(&prop->smallValue_default.ivec4Value, value, sizeof(ivec4));
}

void ShaderProperty_InitDefault_UInt(ShaderProperty *prop, GLuint program, char *name, unsigned int value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_UINT, false);
    prop->smallValue_default.uintValue = value;
}

void ShaderProperty_InitDefault_Mat2(ShaderProperty *prop, GLuint program, char *name, mat2 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_MAT2, false);
    memcpy(&prop->smallValue_default.mat2Value, value, sizeof(mat2));
}

void ShaderProperty_InitDefault_Mat3(ShaderProperty *prop, GLuint program, char *name, mat3 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_MAT3, false);
    memcpy(&prop->smallValue_default.mat3Value, value, sizeof(mat3));
}

void ShaderProperty_InitDefault_Mat4(ShaderProperty *prop, GLuint program, char *name, mat4 value)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_MAT4, false);
    prop->bigValue_default = malloc(sizeof(mat4));
    memcpy(prop->bigValue_default, value, sizeof(mat4));
}

void ShaderProperty_InitDefault_Sampler2D(ShaderProperty *prop, GLuint program, char *name, GLuint textureID)
{
    GLint loc = glGetUniformLocation(program, name);
    ShaderProperty_PartialInit(prop, name, loc, MPT_SAMPLER2D, false);
    prop->smallValue_default.sampler2DValue = textureID;
}