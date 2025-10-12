#include "rendering/material/material-manager.h"
#include "rendering/material/material.h"

static MaterialManager *_manager = NULL;

// -------------------------
// Creation and Freeing
// -------------------------

void MaterialManager_Select(MaterialManager *manager)
{
    _manager = manager;
}

MaterialManager *MaterialManager_Create()
{
    MaterialManager *manager = malloc(sizeof(MaterialManager));
    manager->materials_size = 0;
    manager->materials = NULL;

    // Pre-allocate some space
    manager->materials_size = 10;
    manager->materials = malloc(sizeof(Material *) * manager->materials_size);
    for (int i = 0; i < manager->materials_size; i++)
    {
        manager->materials[i] = NULL;
    }
    return manager;
}

void MaterialManager_Free(MaterialManager *manager)
{
    if (!manager)
    {
        return;
    }
    for (int i = 0; i < manager->materials_size; i++)
    {
        if (!manager->materials[i])
        {
            continue;
        }
        manager->materials[i]->refCount = 0;
        Material_Free(manager->materials[i]);
    }
    free(manager->materials);
    free(manager);
}

// -------------------------
// Functions
// -------------------------

void MaterialManager_Cleanup()
{
    for (int i = 0; i < _manager->materials_size; i++)
    {
        if (_manager->materials[i] == NULL)
            continue;
        if (_manager->materials[i]->refCount <= 0)
        {
            Material_Free(_manager->materials[i]);
            _manager->materials[i] = NULL;
        }
    }
}

void MaterialManager_RegisterMaterial(Material *material)
{
    // Check if any slots are free before-hand
    for (int i = 0; i < _manager->materials_size; i++)
    {
        if (_manager->materials[i] == NULL)
        {
            _manager->materials[i] = material;
            return;
        }
    }
    _manager->materials_size++;
    _manager->materials = realloc(_manager->materials, sizeof(Material *) * _manager->materials_size);
    _manager->materials[_manager->materials_size - 1] = material;
}