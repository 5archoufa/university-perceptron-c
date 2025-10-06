#ifndef MESH_H
#define MESH_H

#include "utilities/math/v3.h"
#include <stddef.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct UV UV;
typedef struct Vertex Vertex;

struct UV
{
    float u, v;
};

struct Vertex
{
    V3 position; // 12 bytes
    V3 normal;   // 12 bytes
    UV uv;       // 8 bytes
    /// @brief Vertex color in RGBA format
    uint32_t color; // 4 bytes
    /// @brief Padding to align the structure to 48 bytes
    uint32_t padding[3]; // 12 bytes
};

typedef struct
{
    // Open GL locations
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    size_t vertex_count;
    Vertex *vertices;
    size_t index_count;
    uint32_t *indices;
    V3 pivot;
    /// @brief Reference count for shared meshes. Do not modify this directly, use Mesh_AddRef and Mesh_Release.
    int refCount;
} Mesh;

Mesh *Mesh_Create(size_t vertexCount, Vertex *vertices, size_t indexCount, uint32_t *indices, V3 pivot);
void Mesh_Free(Mesh *mesh);
Mesh* Mesh_CreateCube(float size);

#endif