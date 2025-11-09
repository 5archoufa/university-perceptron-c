#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

#include <stddef.h>
#include "rendering/mesh/mesh.h"

// -------------------------
// Types
// -------------------------

typedef struct {
    size_t meshes_size;
    Mesh **meshes;
    Mesh *meshDefault_cube;
    Mesh *meshDefault_sphere;
    Mesh *meshDefault_plane;
    Mesh *meshDefault_quad;
} MeshManager;

// -------------------------
// Creation and Freeing
// -------------------------

MeshManager *MeshManager_Create();
void MeshManager_Free(MeshManager *manager);

// -------------------------
// Functions
// -------------------------

void MeshManager_Select(MeshManager* manager);
void MeshManager_RegisterMesh(Mesh *mesh);
void MeshManager_Cleanup();

// -------------------------
// Default Meshes
// -------------------------

Mesh *MeshManager_GetDefaultCube();
Mesh *MeshManager_GetDefaultPlane();
Mesh *MeshManager_GetDefaultQuad();
Mesh *MeshManager_GetDefaultSphere();

#endif
