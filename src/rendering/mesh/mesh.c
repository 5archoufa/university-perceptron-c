#include "rendering/mesh/mesh.h"
#include "rendering/mesh/mesh-manager.h"
#include "physics/mesh_bvh.h"
// C
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
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

/// @param registerMesh If True, the mesh will be registered in the MeshManager for reuse
Mesh *Mesh_Create(bool registerMesh, size_t vertices_size, Vertex *vertices, size_t indices_size, uint32_t *indices, V3 pivot)
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
    mesh->isRegistered = registerMesh;
    if (registerMesh)
    {
        MeshManager_RegisterMesh(mesh);
    }
    return mesh;
}

// -------------------------
// Meshes
// -------------------------

Mesh *Mesh_CreatePlane(V2 meshScale, V2_INT vertexCount, uint32_t color, V2 pivot)
{
    float dx = meshScale.x / (float)(vertexCount.x - 1);
    float dy = meshScale.y / (float)(vertexCount.y - 1);
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
            indices[index++] = (y + 1) * width + (x + 1);
            indices[index++] = (y + 1) * width + x;
            // Second triangle
            indices[index++] = y * width + x;
            indices[index++] = y * width + (x + 1);
            indices[index++] = (y + 1) * width + (x + 1);
        }
    }
    return Mesh_Create(true, vertices_size, vertices, indices_size, indices, (V3){pivot.x, 0.0f, pivot.y});
}

Mesh *Mesh_CreateQuad(V2 meshSize, V2 pivot, uint32_t color)
{
    float dx = meshSize.x;
    float dy = meshSize.y;
    // Offset the quad by the pivot
    float xOffset = -pivot.x * meshSize.x;
    float yOffset = -pivot.y * meshSize.y;

    V3 positions[4] = {
        {xOffset, yOffset, 0.0f}, // Bottom-left
        {+dx + xOffset, yOffset, 0.0f}, // Bottom-right
        {+dx + xOffset, +dy + yOffset, 0.0f}, // Top-right
        {xOffset, +dy + yOffset, 0.0f}  // Top-left
    };

    // UV given the mesh scale
    UV uvs[4] = {
        {0.0f, 0.0f}, // Bottom-left
        {1.0f, 0.0f}, // Bottom-right
        {1.0f, 1.0f}, // Top-right
        {0.0f, 1.0f}  // Top-left
    };
    V3 normal = {0.0f, 0.0f, 1.0f};
    Vertex vertices[4];
    for (int i = 0; i < 4; i++)
    {
        vertices[i].position = positions[i];
        vertices[i].normal = normal;
        vertices[i].uv = uvs[i];
        vertices[i].color = color;
    }
    uint32_t indices[6] = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };

    return Mesh_Create(true, 4, vertices, 6, indices, (V3){pivot.x * meshSize.x, pivot.y * meshSize.y, 0.0f});
}

#include <math.h>
#include <stdint.h>

#define PI 3.14159265359f

Mesh *Mesh_CreateCube(bool isRegistered, V3 meshSize, V3 pivot, uint32_t color)
{
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

    // Compute cube bounds based on pivot
    float xMin = -pivot.x * meshSize.x;
    float yMin = -pivot.y * meshSize.y;
    float zMin = -pivot.z * meshSize.z;
    float xMax = xMin + meshSize.x;
    float yMax = yMin + meshSize.y;
    float zMax = zMin + meshSize.z;

    // Define cube faces based on bounds
    V3 faceVerts[6][4] = {
        // Front (+Z)
        {{xMin, yMin, zMax}, {xMax, yMin, zMax}, {xMax, yMax, zMax}, {xMin, yMax, zMax}},
        // Back (-Z)
        {{xMax, yMin, zMin}, {xMin, yMin, zMin}, {xMin, yMax, zMin}, {xMax, yMax, zMin}},
        // Right (+X)
        {{xMax, yMin, zMax}, {xMax, yMin, zMin}, {xMax, yMax, zMin}, {xMax, yMax, zMax}},
        // Left (-X)
        {{xMin, yMin, zMin}, {xMin, yMin, zMax}, {xMin, yMax, zMax}, {xMin, yMax, zMin}},
        // Top (+Y)
        {{xMin, yMax, zMax}, {xMax, yMax, zMax}, {xMax, yMax, zMin}, {xMin, yMax, zMin}},
        // Bottom (-Y)
        {{xMin, yMin, zMin}, {xMax, yMin, zMin}, {xMax, yMin, zMax}, {xMin, yMin, zMax}}};

    int v = 0, i = 0;
    for (int face = 0; face < 6; face++)
    {
        for (int j = 0; j < 4; j++)
        {
            vertices[v].position = faceVerts[face][j];
            vertices[v].normal = normals[face];
            vertices[v].uv = faceUVs[j];
            vertices[v].color = color;
            v++;
        }

        // Two triangles per face
        indices[i++] = face * 4 + 0;
        indices[i++] = face * 4 + 2;
        indices[i++] = face * 4 + 1;
        indices[i++] = face * 4 + 2;
        indices[i++] = face * 4 + 0;
        indices[i++] = face * 4 + 3;
    }

    return Mesh_Create(isRegistered, 24, vertices, 36, indices, pivot);
}

Mesh *Mesh_CreateSphere(float meshSize, int rings, int sectors, V3 pivot, uint32_t color, bool invertedFaces)
{
    const int totalVertices = rings * sectors;
    Vertex vertices[totalVertices];
    uint32_t indices[(rings - 1) * sectors * 6]; // 2 triangles per quad

    float radius = meshSize * 0.5f; // full diameter is meshSize
    int vertIndex = 0;
    int indIndex = 0;

    // Generate vertices using spherical coordinates
    for (int ring = 0; ring < rings; ring++)
    {
        float theta = (float)ring / (float)(rings - 1) * 3.14159265f; // 0 to PI
        for (int sector = 0; sector < sectors; sector++)
        {
            float phi = (float)sector / (float)sectors * 2.0f * 3.14159265f; // 0 to 2*PI

            float x = radius * sinf(theta) * cosf(phi);
            float y = radius * cosf(theta);
            float z = radius * sinf(theta) * sinf(phi);

            // Apply pivot like cube does
            vertices[vertIndex].position = (V3){
                x - (pivot.x * 2 - 1) * radius, // pivot 0->1 maps to -r..+r
                y - (pivot.y * 2 - 1) * radius,
                z - (pivot.z * 2 - 1) * radius};

            vertices[vertIndex].normal = V3_NORM((V3){x, y, z}); // Proper sphere normal
            if (invertedFaces)
                vertices[vertIndex].normal = V3_SCALE(vertices[vertIndex].normal, -1.0f);
            vertices[vertIndex].uv = (UV){
                (float)sector / (float)(sectors - 1),
                (float)ring / (float)(rings - 1)};
            vertices[vertIndex].color = color;
            vertIndex++;
        }
    }

    // Generate triangular indices
    for (int ring = 0; ring < rings - 1; ring++)
    {
        for (int sector = 0; sector < sectors; sector++)
        {
            int current = ring * sectors + sector;
            int next = ring * sectors + (sector + 1) % sectors;
            int below = (ring + 1) * sectors + sector;
            int belowNext = (ring + 1) * sectors + (sector + 1) % sectors;

            if (invertedFaces)
            {
                // First triangle
                indices[indIndex++] = current;
                indices[indIndex++] = next;
                indices[indIndex++] = below;

                // Second triangle
                indices[indIndex++] = next;
                indices[indIndex++] = belowNext;
                indices[indIndex++] = below;
            }
            else
            {
                // First triangle
                indices[indIndex++] = current;
                indices[indIndex++] = below;
                indices[indIndex++] = next;

                // Second triangle
                indices[indIndex++] = next;
                indices[indIndex++] = below;
                indices[indIndex++] = belowNext;
            }
        }
    }

    return Mesh_Create(true, totalVertices, vertices, indIndex, indices, pivot);
}

Mesh *Mesh_CreateCylinder(float radius, float height, int sectors, V3 pivot, uint32_t color)
{
    const int sideVertices = (sectors + 1) * 2;
    const int capVertices = sectors * 2 + 2;
    const int totalVertices = sideVertices + capVertices;
    const int totalIndices = sectors * 6 + sectors * 6;

    Vertex *vertices = malloc(sizeof(Vertex) * totalVertices);
    uint32_t *indices = malloc(sizeof(uint32_t) * totalIndices);

    if (!vertices || !indices)
    {
        if (vertices)
            free(vertices);
        if (indices)
            free(indices);
        return NULL;
    }

    int vertIndex = 0;
    int indIndex = 0;

    float halfHeight = height * 0.5f;

    float xOffset = (0.5f - pivot.x) * (radius * 2.0f);
    float yOffset = (0.5f - pivot.y) * height;
    float zOffset = (0.5f - pivot.z) * (radius * 2.0f);

    // Generate side vertices (2 rings)
    for (int ring = 0; ring < 2; ring++)
    {
        float y = (ring == 0) ? -halfHeight : halfHeight;
        for (int sector = 0; sector <= sectors; sector++)
        {
            float angle = (float)sector / (float)sectors * 2.0f * PI;
            float x = radius * cosf(angle);
            float z = radius * sinf(angle);

            vertices[vertIndex].position = (V3){
                x + xOffset,
                y + yOffset,
                z + zOffset};
            vertices[vertIndex].normal = V3_NORM((V3){x, 0.0f, z});
            vertices[vertIndex].uv = (UV){(float)sector / (float)sectors, (float)ring};
            vertices[vertIndex].color = color;
            vertIndex++;
        }
    }

    // Generate top cap center
    int topCenterIndex = vertIndex;
    vertices[vertIndex].position = (V3){xOffset, halfHeight + yOffset, zOffset};
    vertices[vertIndex].normal = (V3){0.0f, 1.0f, 0.0f};
    vertices[vertIndex].uv = (UV){0.5f, 0.5f};
    vertices[vertIndex].color = color;
    vertIndex++;

    // Generate top cap rim vertices
    for (int sector = 0; sector < sectors; sector++)
    {
        float angle = (float)sector / (float)sectors * 2.0f * PI;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        vertices[vertIndex].position = (V3){
            x + xOffset,
            halfHeight + yOffset,
            z + zOffset};
        vertices[vertIndex].normal = (V3){0.0f, 1.0f, 0.0f};
        vertices[vertIndex].uv = (UV){(x + radius) / (2.0f * radius), (z + radius) / (2.0f * radius)};
        vertices[vertIndex].color = color;
        vertIndex++;
    }

    // Generate bottom cap center
    int bottomCenterIndex = vertIndex;
    vertices[vertIndex].position = (V3){xOffset, -halfHeight + yOffset, zOffset};
    vertices[vertIndex].normal = (V3){0.0f, -1.0f, 0.0f};
    vertices[vertIndex].uv = (UV){0.5f, 0.5f};
    vertices[vertIndex].color = color;
    vertIndex++;

    // Generate bottom cap rim vertices
    for (int sector = 0; sector < sectors; sector++)
    {
        float angle = (float)sector / (float)sectors * 2.0f * PI;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        vertices[vertIndex].position = (V3){
            x + xOffset,
            -halfHeight + yOffset,
            z + zOffset};
        vertices[vertIndex].normal = (V3){0.0f, -1.0f, 0.0f};
        vertices[vertIndex].uv = (UV){(x + radius) / (2.0f * radius), (z + radius) / (2.0f * radius)};
        vertices[vertIndex].color = color;
        vertIndex++;
    }

    // Generate side indices (quads)
    for (int sector = 0; sector < sectors; sector++)
    {
        int bottom1 = sector;
        int bottom2 = sector + 1;
        int top1 = (sectors + 1) + sector;
        int top2 = (sectors + 1) + sector + 1;

        // First triangle
        indices[indIndex++] = bottom1;
        indices[indIndex++] = bottom2;
        indices[indIndex++] = top1;

        // Second triangle
        indices[indIndex++] = bottom2;
        indices[indIndex++] = top2;
        indices[indIndex++] = top1;
    }

    // Generate top cap indices
    for (int sector = 0; sector < sectors; sector++)
    {
        int next = (sector + 1) % sectors;
        indices[indIndex++] = topCenterIndex;
        indices[indIndex++] = topCenterIndex + 1 + next;
        indices[indIndex++] = topCenterIndex + 1 + sector;
    }

    // Generate bottom cap indices
    for (int sector = 0; sector < sectors; sector++)
    {
        int next = (sector + 1) % sectors;
        indices[indIndex++] = bottomCenterIndex;
        indices[indIndex++] = bottomCenterIndex + 1 + sector;
        indices[indIndex++] = bottomCenterIndex + 1 + next;
    }

    Mesh *result = Mesh_Create(true, vertIndex, vertices, indIndex, indices, pivot);
    free(vertices);
    free(indices);
    return result;
}

Mesh *Mesh_CreateCylinderHardEdged(float radius, float height, int sectors, V3 pivot, uint32_t color)
{
    // Calculate proper vertex and index counts for hard-edged version
    const int sideVertices = sectors * 4; // 4 unique vertices per side quad
    const int capVertices = sectors * 6;  // 3 vertices per triangle, 2 caps
    const int totalVertices = sideVertices + capVertices;
    const int totalIndices = sectors * 6 + sectors * 6; // Side quads + cap triangles

    Vertex *vertices = malloc(sizeof(Vertex) * totalVertices);
    uint32_t *indices = malloc(sizeof(uint32_t) * totalIndices);

    if (!vertices || !indices)
    {
        if (vertices)
            free(vertices);
        if (indices)
            free(indices);
        return NULL;
    }

    int vertIndex = 0;
    int indIndex = 0;

    float halfHeight = height * 0.5f;

    // Calculate pivot offsets
    // pivot (0.5, 0.5, 0.5) = center at origin
    // pivot (0.5, 0, 0.5) = bottom face center at origin
    // pivot (0, 0, 0) = corner at origin
    float xOffset = (0.5f - pivot.x) * (radius * 2.0f);
    float yOffset = (0.5f - pivot.y) * height;
    float zOffset = (0.5f - pivot.z) * (radius * 2.0f);

    // Generate side quads (hard edges - each quad has its own vertices)
    for (int sector = 0; sector < sectors; sector++)
    {
        float angle1 = (float)sector / (float)sectors * 2.0f * PI;
        float angle2 = (float)(sector + 1) / (float)sectors * 2.0f * PI;

        float x1 = radius * cosf(angle1);
        float z1 = radius * sinf(angle1);
        float x2 = radius * cosf(angle2);
        float z2 = radius * sinf(angle2);

        // Calculate face normal for hard edge (use radial normal for cylinder sides)
        V3 centerToP1 = {x1, 0.0f, z1};
        V3 centerToP2 = {x2, 0.0f, z2};
        V3 avgNormal = V3_ADD(centerToP1, centerToP2);
        V3 faceNormal = V3_NORM(avgNormal);

        // Four vertices for this side quad
        V3 quadVerts[4] = {
            {x1 + xOffset, -halfHeight + yOffset, z1 + zOffset}, // Bottom-left
            {x2 + xOffset, -halfHeight + yOffset, z2 + zOffset}, // Bottom-right
            {x2 + xOffset, halfHeight + yOffset, z2 + zOffset},  // Top-right
            {x1 + xOffset, halfHeight + yOffset, z1 + zOffset}   // Top-left
        };

        UV quadUVs[4] = {
            {(float)sector / (float)sectors, 0.0f},
            {(float)(sector + 1) / (float)sectors, 0.0f},
            {(float)(sector + 1) / (float)sectors, 1.0f},
            {(float)sector / (float)sectors, 1.0f}};

        for (int i = 0; i < 4; i++)
        {
            vertices[vertIndex].position = quadVerts[i];
            vertices[vertIndex].normal = faceNormal;
            vertices[vertIndex].uv = quadUVs[i];
            vertices[vertIndex].color = color;
            vertIndex++;
        }

        // Two triangles for this quad
        int baseIndex = sector * 4;
        indices[indIndex++] = baseIndex + 0;
        indices[indIndex++] = baseIndex + 1;
        indices[indIndex++] = baseIndex + 2;
        indices[indIndex++] = baseIndex + 2;
        indices[indIndex++] = baseIndex + 3;
        indices[indIndex++] = baseIndex + 0;
    }

    // Generate top cap triangles (hard edges)
    V3 topCenter = {xOffset, halfHeight + yOffset, zOffset};
    for (int sector = 0; sector < sectors; sector++)
    {
        float angle1 = (float)sector / (float)sectors * 2.0f * PI;
        float angle2 = (float)(sector + 1) / (float)sectors * 2.0f * PI;

        float x1 = radius * cosf(angle1);
        float z1 = radius * sinf(angle1);
        float x2 = radius * cosf(angle2);
        float z2 = radius * sinf(angle2);

        V3 topNormal = {0.0f, 1.0f, 0.0f};

        // Three vertices for this cap triangle
        vertices[vertIndex].position = topCenter;
        vertices[vertIndex].normal = topNormal;
        vertices[vertIndex].uv = (UV){0.5f, 0.5f};
        vertices[vertIndex].color = color;
        int centerIndex = vertIndex++;

        vertices[vertIndex].position = (V3){x1 + xOffset, halfHeight + yOffset, z1 + zOffset};
        vertices[vertIndex].normal = topNormal;
        vertices[vertIndex].uv = (UV){(x1 / radius + 1.0f) * 0.5f, (z1 / radius + 1.0f) * 0.5f};
        vertices[vertIndex].color = color;
        int v1Index = vertIndex++;

        vertices[vertIndex].position = (V3){x2 + xOffset, halfHeight + yOffset, z2 + zOffset};
        vertices[vertIndex].normal = topNormal;
        vertices[vertIndex].uv = (UV){(x2 / radius + 1.0f) * 0.5f, (z2 / radius + 1.0f) * 0.5f};
        vertices[vertIndex].color = color;
        int v2Index = vertIndex++;

        indices[indIndex++] = centerIndex;
        indices[indIndex++] = v1Index;
        indices[indIndex++] = v2Index;
    }

    // Generate bottom cap triangles (hard edges)
    V3 bottomCenter = {xOffset, -halfHeight + yOffset, zOffset};
    for (int sector = 0; sector < sectors; sector++)
    {
        float angle1 = (float)sector / (float)sectors * 2.0f * PI;
        float angle2 = (float)(sector + 1) / (float)sectors * 2.0f * PI;

        float x1 = radius * cosf(angle1);
        float z1 = radius * sinf(angle1);
        float x2 = radius * cosf(angle2);
        float z2 = radius * sinf(angle2);

        V3 bottomNormal = {0.0f, -1.0f, 0.0f};

        // Three vertices for this cap triangle
        vertices[vertIndex].position = bottomCenter;
        vertices[vertIndex].normal = bottomNormal;
        vertices[vertIndex].uv = (UV){0.5f, 0.5f};
        vertices[vertIndex].color = color;
        int centerIndex = vertIndex++;

        vertices[vertIndex].position = (V3){x2 + xOffset, -halfHeight + yOffset, z2 + zOffset};
        vertices[vertIndex].normal = bottomNormal;
        vertices[vertIndex].uv = (UV){(x2 / radius + 1.0f) * 0.5f, (-z2 / radius + 1.0f) * 0.5f};
        vertices[vertIndex].color = color;
        int v1Index = vertIndex++;

        vertices[vertIndex].position = (V3){x1 + xOffset, -halfHeight + yOffset, z1 + zOffset};
        vertices[vertIndex].normal = bottomNormal;
        vertices[vertIndex].uv = (UV){(x1 / radius + 1.0f) * 0.5f, (-z1 / radius + 1.0f) * 0.5f};
        vertices[vertIndex].color = color;
        int v2Index = vertIndex++;

        indices[indIndex++] = centerIndex;
        indices[indIndex++] = v1Index;
        indices[indIndex++] = v2Index;
    }

    Mesh *result = Mesh_Create(true, vertIndex, vertices, indIndex, indices, pivot);
    free(vertices);
    free(indices);
    return result;
}

// -------------------------
// Wireframe Debug Meshes
// -------------------------

Mesh *Mesh_CreateCubeWireframe(V3 size, V3 pivot, uint32_t color)
{
    Vertex vertices[8];

    // Compute cube bounds based on pivot
    float xMin = -pivot.x * size.x;
    float yMin = -pivot.y * size.y;
    float zMin = -pivot.z * size.z;
    float xMax = xMin + size.x;
    float yMax = yMin + size.y;
    float zMax = zMin + size.z;

    // Define 8 cube corners based on bounds
    V3 cubeVerts[8] = {
        {xMin, yMin, zMin}, // 0: Bottom-back-left
        {xMax, yMin, zMin}, // 1: Bottom-back-right
        {xMax, yMin, zMax}, // 2: Bottom-front-right
        {xMin, yMin, zMax}, // 3: Bottom-front-left
        {xMin, yMax, zMin}, // 4: Top-back-left
        {xMax, yMax, zMin}, // 5: Top-back-right
        {xMax, yMax, zMax}, // 6: Top-front-right
        {xMin, yMax, zMax}  // 7: Top-front-left
    };

    for (int i = 0; i < 8; i++)
    {
        vertices[i].position = cubeVerts[i];
        vertices[i].normal = V3_NORM(cubeVerts[i]); // Use position as debug normal
        vertices[i].uv = (UV){0, 0};
        vertices[i].color = color;
    }

    // 36 indices (12 triangles forming cube faces)
    uint32_t indices[36] = {
        // Bottom face
        0, 1, 2, 2, 3, 0,
        // Top face
        4, 7, 6, 6, 5, 4,
        // Front face
        3, 2, 6, 6, 7, 3,
        // Back face
        1, 0, 4, 4, 5, 1,
        // Left face
        0, 3, 7, 7, 4, 0,
        // Right face
        2, 1, 5, 5, 6, 2};

    return Mesh_Create(true, 8, vertices, 36, indices, pivot);
}

Mesh *Mesh_CreateSphereWireframe(float radius, V3 pivot, uint32_t color)
{
    const int rings = 8;
    const int sectors = 12;
    const int totalVertices = rings * sectors;

    Vertex vertices[totalVertices];
    uint32_t indices[rings * sectors * 6]; // 2 triangles per quad

    float x, y, z;
    int vertIndex = 0;
    int indIndex = 0;

    // Generate vertices using spherical coordinates
    for (int ring = 0; ring < rings; ring++)
    {
        float theta = (float)ring / (float)(rings - 1) * 3.14159f; // 0 to PI
        for (int sector = 0; sector < sectors; sector++)
        {
            float phi = (float)sector / (float)sectors * 2.0f * 3.14159f; // 0 to 2*PI

            x = radius * sinf(theta) * cosf(phi);
            y = radius * cosf(theta);
            z = radius * sinf(theta) * sinf(phi);

            vertices[vertIndex].position = (V3){
                x - pivot.x * radius,
                y - pivot.y * radius,
                z - pivot.z * radius};
            vertices[vertIndex].normal = V3_NORM((V3){x, y, z}); // Proper sphere normal
            vertices[vertIndex].uv = (UV){(float)sector / sectors, (float)ring / rings};
            vertices[vertIndex].color = color;
            vertIndex++;
        }
    }

    // Generate triangular indices
    for (int ring = 0; ring < rings - 1; ring++)
    {
        for (int sector = 0; sector < sectors; sector++)
        {
            int current = ring * sectors + sector;
            int next = ring * sectors + (sector + 1) % sectors;
            int below = (ring + 1) * sectors + sector;
            int belowNext = (ring + 1) * sectors + (sector + 1) % sectors;

            // First triangle
            indices[indIndex++] = current;
            indices[indIndex++] = below;
            indices[indIndex++] = next;

            // Second triangle
            indices[indIndex++] = next;
            indices[indIndex++] = below;
            indices[indIndex++] = belowNext;
        }
    }

    return Mesh_Create(true, totalVertices, vertices, indIndex, indices, pivot);
}

Mesh *Mesh_CreateCapsuleWireframe(float radius, float height, V3 pivot, uint32_t color)
{
    const int rings = 6;
    const int sectors = 8;
    const int cylinderRings = 4;
    const int totalVertices = (rings * sectors) + (cylinderRings * sectors); // Hemisphere + cylinder vertices

    Vertex vertices[totalVertices];
    uint32_t indices[totalVertices * 6]; // Rough estimate for triangular mesh

    int vertIndex = 0;
    int indIndex = 0;

    float halfHeight = height * 0.5f;

    // Generate cylinder vertices (middle section)
    for (int ring = 0; ring < cylinderRings; ring++)
    {
        float y = -halfHeight + (float)ring / (float)(cylinderRings - 1) * height;
        for (int sector = 0; sector < sectors; sector++)
        {
            float angle = (float)sector / (float)sectors * 2.0f * 3.14159f;
            float x = radius * cosf(angle);
            float z = radius * sinf(angle);

            vertices[vertIndex].position = (V3){
                x - pivot.x * radius,
                y - pivot.y * height,
                z - pivot.z * radius};
            vertices[vertIndex].normal = V3_NORM((V3){x, 0, z}); // Cylinder normal
            vertices[vertIndex].uv = (UV){(float)sector / sectors, (float)ring / cylinderRings};
            vertices[vertIndex].color = color;
            vertIndex++;
        }
    }

    // Generate hemisphere vertices (top and bottom)
    for (int hemisphere = 0; hemisphere < 2; hemisphere++)
    {
        float yOffset = (hemisphere == 0) ? halfHeight : -halfHeight;
        float yMult = (hemisphere == 0) ? 1.0f : -1.0f;

        for (int ring = 1; ring < rings; ring++) // Skip the equator
        {
            float theta = (float)ring / (float)rings * 3.14159f * 0.5f; // 0 to PI/2
            for (int sector = 0; sector < sectors; sector++)
            {
                float phi = (float)sector / (float)sectors * 2.0f * 3.14159f;

                float x = radius * sinf(theta) * cosf(phi);
                float y = yOffset + yMult * radius * cosf(theta);
                float z = radius * sinf(theta) * sinf(phi);

                vertices[vertIndex].position = (V3){
                    x - pivot.x * radius,
                    y - pivot.y * height,
                    z - pivot.z * radius};
                vertices[vertIndex].normal = V3_NORM((V3){x, yMult * radius * cosf(theta), z});
                vertices[vertIndex].uv = (UV){(float)sector / sectors, (float)ring / rings};
                vertices[vertIndex].color = color;
                vertIndex++;
            }
        }
    }

    // Generate triangular indices for cylinder
    for (int ring = 0; ring < cylinderRings - 1; ring++)
    {
        for (int sector = 0; sector < sectors; sector++)
        {
            int current = ring * sectors + sector;
            int next = ring * sectors + (sector + 1) % sectors;
            int below = (ring + 1) * sectors + sector;
            int belowNext = (ring + 1) * sectors + (sector + 1) % sectors;

            // First triangle
            indices[indIndex++] = current;
            indices[indIndex++] = below;
            indices[indIndex++] = next;

            // Second triangle
            indices[indIndex++] = next;
            indices[indIndex++] = below;
            indices[indIndex++] = belowNext;
        }
    }

    return Mesh_Create(true, vertIndex, vertices, indIndex, indices, pivot);
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
    if (!mesh->isRegistered) // Register the mesh to be freed automatically
    {
        MeshManager_RegisterMesh(mesh);
    }
    if (mesh->refCount < 0)
    {
        LogWarning(&_logConfig, "Mesh_MarkUnreferenced: mesh refCount < 0");
        mesh->refCount = 0;
    }
}

// -------------------------
// Utilities
// -------------------------

/// @brief Helper function to update GPU buffers
void Mesh_UpdateIndexBuffer(Mesh *mesh, size_t offset, size_t count, const uint32_t *data)
{
    if (!mesh || !mesh->EBO || !data)
        return;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
                    offset * sizeof(uint32_t), 
                    count * sizeof(uint32_t), 
                    data);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/// @brief Helper function to update GPU buffers
void Mesh_UpdateVertexBuffer(Mesh *mesh, size_t offset, size_t count, const Vertex *data)
{
    if (!mesh || !mesh->VBO || !data)
        return;
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 
                    offset * sizeof(Vertex), 
                    count * sizeof(Vertex), 
                    data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

inline void Vertex_MinMax(size_t vertices_size, Vertex *vertices, V3 *min, V3 *max)
{
    *min = vertices[0].position;
    *max = vertices[0].position;
    for (int i = 1; i < vertices_size; i++)
    {
        *min = V3_MIN(*min, vertices[i].position);
        *max = V3_MAX(*max, vertices[i].position);
    }
}