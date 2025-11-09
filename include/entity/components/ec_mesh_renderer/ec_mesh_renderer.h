#ifndef EC_MESH_RENDERER_H
#define EC_MESH_RENDERER_H

// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// Mesh
#include "rendering/mesh/mesh.h"
// Material
#include "rendering/material/material.h"
// Texture
#include "rendering/texture/texture.h"
// Math
#include <cglm/cglm.h>
// C
#include <stdint.h>
// AABB
#include "physics/aabb.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct EC_MeshRenderer EC_MeshRenderer;
typedef struct World World;
typedef struct Component Component;

struct EC_MeshRenderer{
    Component* component;
    Mesh* mesh;
    V3 meshScale;
    Material* material;
    AABB bounds;
};

// -------------------------
// Creation and Freeing
// -------------------------

EC_MeshRenderer *EC_MeshRenderer_Create(Entity *entity, Mesh *mesh, V3 meshScale, Material *material);

// -------------------------
// Materials
// -------------------------

void EC_MeshRenderer_SetDefaultMaterial(Material *material);

// -------------------------
// Prefabs
// -------------------------

EC_MeshRenderer *Prefab_Quad(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V2 meshSize, uint32_t vertexColor, Material* material);
EC_MeshRenderer *Prefab_Cube(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshSize, uint32_t vertexColor);
Entity *Prefab_CubeWCollider(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshSize, uint32_t vertexColor);
EC_MeshRenderer *Prefab_Sphere(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);
EC_MeshRenderer *Prefab_PlaneDefault(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);
EC_MeshRenderer *Prefab_CubeDefault(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);

// -------------------------
// Bounds
// -------------------------
void EC_MeshRenderer_CalculateBounds(EC_MeshRenderer *ec_meshRenderer);


#endif