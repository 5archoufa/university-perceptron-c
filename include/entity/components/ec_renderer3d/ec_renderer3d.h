#ifndef RENDERER3D_H
#define RENDERER3D_H

#include "entity/entity.h"
#include "rendering/mesh/mesh.h"
#include "rendering/material/material.h"
#include "rendering/texture/texture.h"
#include <cglm/cglm.h>
#include <stdint.h>
#include "entity/transform.h"

typedef struct EC_Renderer3D EC_Renderer3D;
typedef struct World World;

struct EC_Renderer3D{
    Component* component;
    Mesh* mesh;
    Material* material;
    Bounds bounds;
};

// -------------------------
// Creation and Freeing
// -------------------------
EC_Renderer3D* EC_Renderer3D_Create(Entity* entity, Mesh* mesh, Material* material);

// -------------------------
// Materials
// -------------------------
void EC_Renderer3D_SetDefaultMaterial(Material *material);

// -------------------------
// Prefabs
// -------------------------
EC_Renderer3D *Prefab_Cube(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float meshSize, Texture *texture);

// -------------------------
// Bounds
// -------------------------
void EC_Renderer3D_CalculateBounds(EC_Renderer3D *ec_renderer3d);


#endif