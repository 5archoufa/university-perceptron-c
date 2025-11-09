#ifndef MESH_H
#define MESH_H

#include "utilities/math/v3.h"
#include <stddef.h>
#include <stdbool.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Forward declaration for BVH
typedef struct MeshBVH MeshBVH;

// ------------------------- 
// Types 
// -------------------------

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
    uint32_t id;
    // Open GL locations
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    size_t vertices_size;
    Vertex *vertices;
    size_t indices_size;
    uint32_t *indices;
    V3 pivot;
    /// @brief Reference count for shared meshes. Do not modify this directly, use Mesh_AddRef and Mesh_Release.
    bool isRegistered;
    int refCount;
} Mesh;

// ------------------------- 
// Creation & Freeing 
// -------------------------

Mesh *Mesh_Create(bool registerMesh, size_t vertexCount, Vertex *vertices, size_t indexCount, uint32_t *indices, V3 pivot);
void Mesh_Free(Mesh *mesh);

// ------------------------- 
// Meshes 
// -------------------------

Mesh *Mesh_CreateCube(bool isRegistered, V3 meshSize, V3 pivot, uint32_t color);
Mesh *Mesh_CreatePlane(V2 meshScale, V2_INT vertexCount, uint32_t color, V2 pivot);
Mesh *Mesh_CreateQuad(V2 meshSize, V2 pivot, uint32_t color);
Mesh *Mesh_CreateSphere(float meshSize, int rings, int sectors, V3 pivot, uint32_t color, bool invertedFaces);
Mesh *Mesh_CreateCylinder(float radius, float height, int sectors, V3 pivot, uint32_t color);
Mesh *Mesh_CreateCylinderHardEdged(float radius, float height, int sectors, V3 pivot, uint32_t color);

// Wireframe debug meshes
Mesh *Mesh_CreateCubeWireframe(V3 size, V3 pivot, uint32_t color);
Mesh *Mesh_CreateSphereWireframe(float radius, V3 pivot, uint32_t color);
Mesh *Mesh_CreateCapsuleWireframe(float radius, float height, V3 pivot, uint32_t color);

// ------------------------- 
// Tracking 
// -------------------------

void Mesh_MarkReferenced(Mesh *mesh);
void Mesh_MarkUnreferenced(Mesh *mesh);

// ------------------------- 
// Utilities 
// -------------------------

void Mesh_UpdateVertexBuffer(Mesh *mesh, size_t offset, size_t count, const Vertex *data);
void Mesh_UpdateIndexBuffer(Mesh *mesh, size_t offset, size_t count, const uint32_t *data);
void Vertex_MinMax(size_t vertices_size, Vertex* vertices, V3 *min, V3 *max);

#endif