#include "rendering/mesh/mesh.h"
#include "rendering/mesh/mesh-manager.h"
// C
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
// Logging
#include "logging/logger.h"

// -------------------------
// Static
// -------------------------

static LogConfig _logConfig = {"Mesh", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Creation & Freeing
// -------------------------

void Mesh_Free(Mesh *mesh)
{
    if (mesh->refCount > 0)
    {
        Log(&_logConfig, "Could not free Mesh: refCount %d != 0\n", mesh->refCount);
        return;
    }
    Log(&_logConfig, "Freeing VAO: %u, VBO: %u, EBO: %u\n", mesh->VAO, mesh->VBO, mesh->EBO);
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);
    free(mesh->vertices);
    free(mesh->indices);
    free(mesh);
}

Mesh *Mesh_Create(size_t vertices_size, Vertex *vertices, size_t indices_size, uint32_t *indices, V3 pivot)
{
    Mesh *mesh = malloc(sizeof(Mesh));
    // Vertices
    mesh->vertices_size = vertices_size;
    mesh->vertices = malloc(sizeof(Vertex) * vertices_size);
    for (int i = 0; i < vertices_size; i++)
    {
        mesh->vertices[i] = vertices[i];
    }
    // Indices
    mesh->indices_size = indices_size;
    mesh->indices = malloc(indices_size * sizeof(uint32_t));
    for (int i = 0; i < indices_size; i++)
    {
        mesh->indices[i] = indices[i];
    }
    // Pivot
    mesh->pivot = pivot;
    // Referencing
    mesh->refCount = 0;
    // Generate OpenGL objects
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);
    // Bind VAO (all vertex attribute state will be stored in this VAO)
    glBindVertexArray(mesh->VAO);
    // Upload vertices (use vertex_count * sizeof(Vertex))
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_size * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);
    // Upload indices (use index_count * sizeof(uint32_t))
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_size * sizeof(uint32_t), mesh->indices, GL_STATIC_DRAW);
    // Set vertex attributes
    // Position (location = 0) -> 3 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    // Normal (location = 1) -> 3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    // UV (location = 2) -> 2 floats
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
    // Color (location = 3) -> 4 unsigned bytes normalized to float0..1
    // If color is a packed uint32_t (RGBA), this interprets it as 4 bytes (A,B,G,R) depending on endianness.
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));
    // Unbind VAO
    glBindVertexArray(0);
    // Register Mesh
    MeshManager_RegisterMesh(mesh);
    return mesh;
}

// -------------------------
// Meshes
// -------------------------

Mesh *Mesh_CreatePlane(V2 meshScale, V2_INT vertexCount, uint32_t color, V2 pivot)
{
    float dx = meshScale.x / (float)vertexCount.x;
    float dy = meshScale.y / (float)vertexCount.y;
    float xOffset = -pivot.x * meshScale.x;
    float yOffset = -pivot.y * meshScale.y;
    int width = vertexCount.x;
    int height = vertexCount.y;
    // Vertices
    uint32_t vertices_size = width * height;
    Vertex *vertices = malloc(sizeof(Vertex) * vertices_size);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            vertices[y * width + x].position = (V3){
                x * dx + xOffset,
                0.0f,
                y * dy + yOffset};
            vertices[y * width + x].normal = (V3){0.0, 1.0, 0.0};
            vertices[y * width + x].uv = (UV){
                (float)x / (float)(width - 1),
                (float)y / (float)(height - 1)};
            vertices[y * width + x].color = color;
        }
    }
    // Indices
    uint32_t indices_size = (width - 1) * (height - 1) * 6;
    uint32_t *indices = malloc(sizeof(uint32_t) * indices_size);
    int index = 0;
    for (int y = 0; y < height - 1; y++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            // First triangle
            indices[index++] = y * width + x;
            indices[index++] = (y + 1) * width + x;
            indices[index++] = (y + 1) * width + (x + 1);
            // Second triangle
            indices[index++] = y * width + x;
            indices[index++] = (y + 1) * width + (x + 1);
            indices[index++] = y * width + (x + 1);
        }
    }
    return Mesh_Create(vertices_size, vertices, indices_size, indices, (V3){pivot.x * meshScale.x, 0.0f, pivot.y * meshScale.y});
}

Mesh *Mesh_CreateCube(V3 meshSize, V3 pivot, uint32_t color)
{
    float dx = meshSize.x * 0.5f;
    float dy = meshSize.y * 0.5f;
    float dz = meshSize.z * 0.5f;

    // Offset the cube by the pivot
    float xOffset = -pivot.x * meshSize.x;
    float yOffset = -pivot.y * meshSize.y;
    float zOffset = -pivot.z * meshSize.z;

    Vertex vertices[24];
    uint32_t indices[36];

    // Define cube faces
    V3 normals[6] = {
        {0, 0, 1},  // Front
        {0, 0, -1}, // Back
        {1, 0, 0},  // Right
        {-1, 0, 0}, // Left
        {0, 1, 0},  // Top
        {0, -1, 0}  // Bottom
    };

    // UVs (same for all faces)
    UV faceUVs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}};

    // Positions for each face (before scaling)
    V3 faceVerts[6][4] = {
        // Front (+Z)
        {{-dx, -dy, +dz}, {+dx, -dy, +dz}, {+dx, +dy, +dz}, {-dx, +dy, +dz}},
        // Back (-Z)
        {{+dx, -dy, -dz}, {-dx, -dy, -dz}, {-dx, +dy, -dz}, {+dx, +dy, -dz}},
        // Right (+X)
        {{+dx, -dy, +dz}, {+dx, -dy, -dz}, {+dx, +dy, -dz}, {+dx, +dy, +dz}},
        // Left (-X)
        {{-dx, -dy, -dz}, {-dx, -dy, +dz}, {-dx, +dy, +dz}, {-dx, +dy, -dz}},
        // Top (+Y)
        {{-dx, +dy, +dz}, {+dx, +dy, +dz}, {+dx, +dy, -dz}, {-dx, +dy, -dz}},
        // Bottom (-Y)
        {{-dx, -dy, -dz}, {+dx, -dy, -dz}, {+dx, -dy, +dz}, {-dx, -dy, +dz}}};

    int v = 0, i = 0;
    for (int face = 0; face < 6; face++)
    {
        for (int j = 0; j < 4; j++)
        {
            vertices[v].position = (V3){
                faceVerts[face][j].x + xOffset,
                faceVerts[face][j].y + yOffset,
                faceVerts[face][j].z + zOffset};
            vertices[v].normal = normals[face];
            vertices[v].uv = faceUVs[j];
            vertices[v].color = color;
            v++;
        }

        // Two triangles per face
        indices[i++] = face * 4 + 0;
        indices[i++] = face * 4 + 1;
        indices[i++] = face * 4 + 2;
        indices[i++] = face * 4 + 2;
        indices[i++] = face * 4 + 3;
        indices[i++] = face * 4 + 0;
    }

    return Mesh_Create(24, vertices, 36, indices, pivot);
}

// -------------------------
// Tracking
// -------------------------

void Mesh_MarkReferenced(Mesh *mesh)
{
    mesh->refCount++;
}

void Mesh_MarkUnreferenced(Mesh *mesh)
{
    mesh->refCount--;
    if (mesh->refCount < 0)
    {
        LogWarning(&_logConfig, "Mesh_MarkUnreferenced: mesh refCount < 0");
        mesh->refCount = 0;
    }
}

// ------------------------- 
// Utilities 
// -------------------------

inline void Vertex_MinMax(size_t vertices_size, Vertex* vertices, V3 *min, V3 *max){
    *min = vertices[0].position;
    *max = vertices[0].position;
    for(int i = 1;i<vertices_size;i++){
        *min = V3_MIN(*min, vertices[i].position);
        *max = V3_MAX(*max, vertices[i].position);
    }
}