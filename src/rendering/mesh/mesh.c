#include "rendering/mesh/mesh.h"
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

static const uint32_t _CUBE_INDICES[36] = {
    // Each face uses counter-clockwise winding when viewed from outside
    0, 3, 2, 2, 1, 0, // back face  (Z-)
    4, 5, 6, 6, 7, 4, // front face (Z+)
    4, 0, 1, 1, 5, 4, // bottom face (Y-)
    6, 2, 3, 3, 7, 6, // top face (Y+)
    4, 7, 3, 3, 0, 4, // left face (X-)
    1, 2, 6, 6, 5, 1  // right face (X+)
};
static const V3 _CUBE_VERTICES[8] = {
    {-0.5f, -0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    {-0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f}};

void Mesh_Free(Mesh *mesh)
{
    printf("Freeing VAO: %u, VBO: %u, EBO: %u\n", mesh->VAO, mesh->VBO, mesh->EBO);
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);
    if (mesh->vertices)
        free(mesh->vertices);
    if (mesh->indices)
        free(mesh->indices);
    free(mesh);
}

Mesh *Mesh_CreatePlane(V2 meshScale, V2_INT vertexCount, uint32_t color)
{
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
                x * meshScale.x,
                0.0f,
                y * meshScale.y};
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
    return Mesh_Create(vertices_size, vertices, indices_size, indices, V3_ZERO);
}

Mesh *Mesh_Create(size_t vertexCount, Vertex *vertices, size_t indexCount, uint32_t *indices, V3 pivot)
{
    Mesh *mesh = malloc(sizeof(Mesh));
    mesh->vertex_count = vertexCount;
    mesh->vertices = vertices;
    mesh->index_count = indexCount;
    mesh->indices = indices;
    mesh->pivot = pivot;
    mesh->refCount = 0;

    // Generate objects
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    // Bind VAO (all vertex attribute state will be stored in this VAO)
    glBindVertexArray(mesh->VAO);

    // Upload vertices (use vertex_count * sizeof(Vertex))
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);

    // Upload indices (use index_count * sizeof(uint32_t))
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(uint32_t), mesh->indices, GL_STATIC_DRAW);

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
    // If you want to treat color as integer: glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // Unbind VAO (optional)
    glBindVertexArray(0);

    MeshManager_AddMesh(mesh);
    return mesh;
}

Mesh *Mesh_CreateCube(float size)
{
    Vertex *vertices = malloc(sizeof(Vertex) * 8);
    uint32_t colors[8] = {
        0xFFFF0000, // Red
        0xFF00FF00, // Green
        0xFF0000FF, // Blue
        0xFFFFFF00, // Yellow
        0xFFFF00FF, // Magenta
        0xFF00FFFF, // Cyan
        0xFFFFFFFF, // White
        0xFF808080  // Gray
    };
    for (int i = 0; i < 8; i++)
    {
        vertices[i].position = V3_SCALE(_CUBE_VERTICES[i], size);
        vertices[i].normal = V3_ZERO;    // Normals can be computed later if needed
        vertices[i].uv = (UV){0.0, 0.0}; // UVs can be set later if needed
        vertices[i].color = colors[i];   // Default color
    }
    uint32_t *indices = malloc(sizeof(uint32_t) * 36);
    for (int i = 0; i < 36; i++)
    {
        indices[i] = _CUBE_INDICES[i];
    }
    return Mesh_Create(8, vertices, 36, indices, V3_HALF);
}