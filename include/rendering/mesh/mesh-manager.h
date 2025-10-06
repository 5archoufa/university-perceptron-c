#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

// -------------------------
// Types
// -------------------------

typedef struct MeshManager MeshManager;
typedef struct Mesh Mesh;

struct MeshManager{
    size_t meshes_size;
    Mesh **meshes;
};

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
