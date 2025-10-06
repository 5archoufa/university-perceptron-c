#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

// -------------------------
// Types
// -------------------------

typedef struct Material Material;
typedef struct MaterialManager MaterialManager;

struct MaterialManager{
    size_t materials_size;
    Material **materials;
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
void MaterialManager_AddMaterial(Material *material);
void MaterialManager_Cleanup();

#endif