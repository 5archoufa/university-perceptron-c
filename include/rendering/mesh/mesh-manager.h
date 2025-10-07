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
void MeshManager_AddMesh(Mesh *mesh);
void MeshManager_Cleanup();

#endif
