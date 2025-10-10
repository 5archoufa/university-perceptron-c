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

inline static void ShaderProperty_PartialInit(ShaderProperty *prop, const char *name, GLint loc, ShaderPropertyType type, bool isBig)
{
    prop->name = malloc(sizeof(char) * 64);
    strcpy(prop->name, name);
    prop->loc = loc;
    prop->type = type;
    prop->isBig = isBig;
    prop->bigValue_default = (ShaderPropBigValue){
        .value = NULL,
        .size = 0
    };
    prop->bigValue_previous = (ShaderPropBigValue){
        .value = NULL,
        .size = 0
    };
}

// -------------------------
// Creation and Freeing
// -------------------------

Shader* Shader_LoadFromFile(char *name, char *vertexPath, char *fragmentPath, size_t properties_size)
{
    // Load shader source code from file
    char *vertexSource = File_LoadStr(vertexPath);
    char *fragmentSource = File_LoadStr(fragmentPath);

    // Create shader
    Shader* shader = Shader_Create(name, vertexSource, fragmentSource, properties_size);

    // Free loaded source code
    free(vertexSource);
    free(fragmentSource);
    return shader;
}

Shader* Shader_Create(const char *name, const char *vertexSource, const char *fragmentSource, size_t properties_size)
{
    GLint shaderProgram = Shader_CreateShaderProgram(vertexSource, fragmentSource);
    if (shaderProgram == 0)
    {
        LogError(&_logConfig, "Error Creating Shader %s", name);
    }
    Shader *shader = malloc(sizeof(Shader));
    shader->shaderProgram = shaderProgram;
    // Name
    shader->name = malloc(sizeof(char) * 64);
    strcpy(shader->name, name);
    // Properties
    shader->properties_size = properties_size;
    shader->properties = malloc(sizeof(ShaderProperty) * properties_size);
    // Model Location
    shader->modelLoc = glGetUniformLocation(shaderProgram, "model");
    // Add to shader list
    ShaderManager_AddShader(shader);
    return shader;
}

void Shader_Free(Shader *shader)
{
    glDeleteProgram(shader->shaderProgram);
    for (int i = 0; i < shader->properties_size; i++)
    {
        free(shader->properties[i].name);
        if (shader->properties[i].isBig)
        {
            free(shader->properties[i].bigValue_default.value);
            free(shader->properties[i].bigValue_previous.value);
        }
    }
    free(shader->name);
    free(shader->properties);
    free(shader);
}

// -------------------------
// Property Initializers
// -------------------------

void ShaderProperty_InitDefault_Float(Shader* shader, int index, const char *name, float value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_FLOAT, false);
    shader->properties[index].smallValue_default.floatValue = value;
}

void ShaderProperty_InitDefault_Vec2(Shader* shader, int index, const char *name, vec2 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_VEC2, false);
    memcpy(&shader->properties[index].smallValue_default.vec2Value, value, sizeof(vec2));
}

void ShaderProperty_InitDefault_Vec3(Shader* shader, int index, const char *name, vec3 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_VEC3, false);
    memcpy(&shader->properties[index].smallValue_default.vec3Value, value, sizeof(vec3));
}

void ShaderProperty_InitDefault_Vec4(Shader* shader, int index, const char *name, vec4 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_VEC4, false);
    memcpy(&shader->properties[index].smallValue_default.vec4Value, value, sizeof(vec4));
}

void ShaderProperty_InitDefault_Int(Shader* shader, int index, const char *name, int value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_INT, false);
    shader->properties[index].smallValue_default.intValue = value;
}

void ShaderProperty_InitDefault_IVec2(Shader* shader, int index, const char *name, ivec2 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_IVEC2, false);
    memcpy(&shader->properties[index].smallValue_default.ivec2Value, value, sizeof(ivec2));
}

void ShaderProperty_InitDefault_IVec3(Shader* shader, int index, const char *name, ivec3 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_IVEC3, false);
    memcpy(&shader->properties[index].smallValue_default.ivec3Value, value, sizeof(ivec3));
}

void ShaderProperty_InitDefault_IVec4(Shader* shader, int index, const char *name, ivec4 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_IVEC4, false);
    memcpy(&shader->properties[index].smallValue_default.ivec4Value, value, sizeof(ivec4));
}

void ShaderProperty_InitDefault_UInt(Shader* shader, int index, const char *name, unsigned int value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_UINT, false);
    shader->properties[index].smallValue_default.uintValue = value;
}

void ShaderProperty_InitDefault_Mat2(Shader* shader, int index, const char *name, mat2 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_MAT2, false);
    memcpy(&shader->properties[index].smallValue_default.mat2Value, value, sizeof(mat2));
}

void ShaderProperty_InitDefault_Mat3(Shader* shader, int index, const char *name, mat3 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_MAT3, false);
    memcpy(&shader->properties[index].smallValue_default.mat3Value, value, sizeof(mat3));
}

void ShaderProperty_InitDefault_Sampler2D(Shader* shader, int index, const char *name, GLuint textureID)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_SAMPLER2D, false);
    shader->properties[index].smallValue_default.sampler2DValue = textureID;
}

void ShaderProperty_InitDefault_Mat4(Shader* shader, int index, const char *name, mat4 value)
{
    GLint loc = glGetUniformLocation(shader->shaderProgram, name);
    ShaderProperty_PartialInit(&shader->properties[index], name, loc, MPT_MAT4, false);
    shader->properties[index].bigValue_default = (ShaderPropBigValue){
        .size = sizeof(mat4),
        .value = malloc(sizeof(mat4)),
    };
    memcpy(shader->properties[index].bigValue_default.value, value, sizeof(mat4));
}