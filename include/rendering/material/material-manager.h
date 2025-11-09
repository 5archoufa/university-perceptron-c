#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

// C
#include <stddef.h>
#include <stdint.h>

// -------------------------
// Types
// -------------------------

typedef struct Material Material;
typedef struct MaterialManager MaterialManager;

struct MaterialManager{
    size_t materials_size;
    Material **materials;
    uint32_t nextMaterialId;
};

// -------------------------
// Creation and Freeing
// -------------------------

MaterialManager *MaterialManager_Create();
void MaterialManager_Free(MaterialManager *manager);

// -------------------------
// Functions
// -------------------------

void MaterialManager_Select(MaterialManager* manager);
void MaterialManager_RegisterMaterial(Material *material);
void MaterialManager_Cleanup();

#endif