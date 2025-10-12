#include "rendering/mesh/mesh.h"
#include "rendering/mesh/mesh-manager.h"

#include <stdlib.h>

// -------------------------
// Creation and Freeing
// -------------------------

static MeshManager *_manager = NULL;

MeshManager *MeshManager_Create()
{
    MeshManager *manager = malloc(sizeof(MeshManager));
    manager->meshes_size = 0;
    manager->meshes = NULL;

    // Pre-allocate some space
    manager->meshes_size = 10;
    manager->meshes = malloc(sizeof(Mesh *) * manager->meshes_size);
    for (int i = 0; i < manager->meshes_size; i++)
    {
        manager->meshes[i] = NULL;
    }
    return manager;
}

void MeshManager_Free(MeshManager *manager)
{
    if (!manager)
    {
        return;
    }
    for (int i = 0; i < manager->meshes_size; i++)
    {
        if (!manager->meshes[i])
        {
            continue;
        }
        manager->meshes[i]->refCount = 0;
        Mesh_Free(manager->meshes[i]);
    }
    free(manager->meshes);
    free(manager);
}

// -------------------------
// Functions
// -------------------------

void MeshManager_Select(MeshManager *manager)
{
    _manager = manager;
}

void MeshManager_Cleanup()
{
    for (int i = 0; i < _manager->meshes_size; i++)
    {
        if (_manager->meshes[i] == NULL)
            continue;
        if (_manager->meshes[i]->refCount <= 0)
        {
            Mesh_Free(_manager->meshes[i]);
            _manager->meshes[i] = NULL;
        }
    }
}

void MeshManager_RegisterMesh(Mesh *mesh)
{
    // Check if any slots are free before-hand
    for (int i = 0; i < _manager->meshes_size; i++)
    {
        if (_manager->meshes[i] == NULL)
        {
            _manager->meshes[i] = mesh;
            return;
        }
    }
    _manager->meshes_size++;
    _manager->meshes = realloc(_manager->meshes, sizeof(Mesh *) * _manager->meshes_size);
    _manager->meshes[_manager->meshes_size - 1] = mesh;
}
