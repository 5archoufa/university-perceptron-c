#include "rendering/mesh/mesh.h"
#include "rendering/mesh/mesh-manager.h"
// C
#include <stdlib.h>
// Logging
#include "logging/logger.h"

// ------------------------- 
// Static Variables 
// -------------------------

static LogConfig _logConfig = {"MeshManager", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

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
    // Select this manager as the current one
    MeshManager_Select(manager);
    // Create default meshes
    manager->meshDefault_cube = Mesh_CreateCube(false, V3_ONE, V3_HALF, 0xFFFFFFFF);
    Mesh_MarkReferenced(manager->meshDefault_cube);
    manager->meshDefault_plane = Mesh_CreatePlane(V2_ONE, (V2_INT){2, 2}, 0xFFFFFFFF, V2_HALF);
    Mesh_MarkReferenced(manager->meshDefault_plane);
    manager->meshDefault_quad = Mesh_CreateQuad(V2_ONE, V2_HALF, 0xFFFFFFFF);
    Mesh_MarkReferenced(manager->meshDefault_quad);
    manager->meshDefault_sphere = Mesh_CreateSphere(1.0f, 13, 15, V3_HALF, 0xFFFFFFFF, false);
    Mesh_MarkReferenced(manager->meshDefault_sphere);
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
// Default Meshes 
// -------------------------

Mesh *MeshManager_GetDefaultSphere()
{
    return _manager->meshDefault_sphere;
}

Mesh *MeshManager_GetDefaultCube()
{
    return _manager->meshDefault_cube;
}

Mesh *MeshManager_GetDefaultPlane()
{
    return _manager->meshDefault_plane;
}

Mesh *MeshManager_GetDefaultQuad()
{
    return _manager->meshDefault_quad;
}

// -------------------------
// Functions
// -------------------------

Mesh *MeshManager_GetMesh(uint32_t id)
{
    return _manager->meshes[id];
}

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
            Log(&_logConfig, "Cleaning up unreferenced Mesh ID %d", i);
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
            mesh->id = i;
            return;
        }
    }
    _manager->meshes_size++;
    int id = _manager->meshes_size - 1;
    _manager->meshes = realloc(_manager->meshes, sizeof(Mesh *) * _manager->meshes_size);
    _manager->meshes[id] = mesh;
    mesh->id = id;
}
